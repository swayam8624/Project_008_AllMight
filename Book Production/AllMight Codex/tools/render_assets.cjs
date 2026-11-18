#!/usr/bin/env node
'use strict';

const fs = require('node:fs/promises');
const path = require('node:path');
const crypto = require('node:crypto');
const playwrightModule = process.env.PLAYWRIGHT_MODULE || 'playwright';
const { chromium } = require(playwrightModule);

const CHROME = '/Applications/Google Chrome.app/Contents/MacOS/Google Chrome';
const MATHJAX_VERSION = '3.2.2';
const MERMAID_VERSION = '11';
const RENDERER_VERSION = `mathjax-${MATHJAX_VERSION}-tex-svg-tex-mml-fontCache-none-pt-dimensions-error-detect-v4+mermaid-${MERMAID_VERSION}-intrinsic-dimensions-v2`;
const MAX_MATH_PAGES = 4;
const DEFAULT_TIMEOUT_MS = 60000;
const MATH_EX_PT = 5.5;
const MAX_LAYOUT_WIDTH_PT = 396;
const MAX_LAYOUT_HEIGHT_PT = 500;

function sha256(text) {
  return crypto.createHash('sha256').update(text).digest('hex');
}

function stableAssetHash(asset) {
  const payload = {
    id: asset.id,
    kind: asset.kind,
    tex: asset.tex ?? null,
    code: asset.code ?? null,
    display: asset.display ?? null,
    rendererVersion: asset.kind === 'math'
      ? `MathJax-${MATHJAX_VERSION}-tex-svg-fontCache-none`
      : `Mermaid-${MERMAID_VERSION}`,
  };
  // Keep the historical hash payload byte-for-byte for ordinary jobs so
  // adding normalization support does not invalidate the valid asset cache.
  if (asset.kind === 'math' && typeof asset.render_tex === 'string') {
    payload.render_tex = asset.render_tex;
    payload.normalization_protocol = asset.normalization_protocol || 'render-normalization-v1';
  }
  if (asset.kind === 'mermaid' && typeof asset.render_code === 'string') {
    payload.render_code = asset.render_code;
    payload.normalization_protocol = asset.normalization_protocol || 'render-normalization-v1';
  }
  return sha256(JSON.stringify(payload));
}

function assertJob(asset, index) {
  if (!asset || typeof asset !== 'object' || Array.isArray(asset)) {
    throw new Error(`jobs[${index}] must be an object`);
  }
  if (typeof asset.id !== 'string' || !asset.id.trim()) {
    throw new Error(`jobs[${index}].id must be a nonempty string`);
  }
  if (asset.kind !== 'math' && asset.kind !== 'mermaid') {
    throw new Error(`jobs[${index}].kind must be 'math' or 'mermaid'`);
  }
  const body = asset.kind === 'math' ? asset.tex : asset.code;
  if (typeof body !== 'string' || !body.trim()) {
    throw new Error(`jobs[${index}] requires nonempty ${asset.kind === 'math' ? 'tex' : 'code'}`);
  }
  if (typeof asset.outfile !== 'string' || !asset.outfile.trim()) {
    throw new Error(`jobs[${index}].outfile must be a nonempty path`);
  }
  if (asset.kind === 'math' && asset.display !== undefined && typeof asset.display !== 'boolean') {
    throw new Error(`jobs[${index}].display must be a boolean when provided`);
  }
}

function attr(tag, name) {
  const escaped = name.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  const m = tag.match(new RegExp(`(?:^|\\s)${escaped}="([^"]*)"`));
  return m ? m[1] : null;
}

function parseViewBox(viewBox) {
  if (!viewBox) return null;
  const values = viewBox.trim().split(/[ ,]+/).map(Number);
  if (values.length !== 4 || values.some((value) => !Number.isFinite(value))) return null;
  return { x: values[0], y: values[1], width: values[2], height: values[3] };
}

function lengthToPt(value) {
  if (!value) return null;
  const match = value.trim().match(/^([0-9]+(?:\.[0-9]+)?)(ex|pt|px)?$/i);
  if (!match) return null;
  const number = Number(match[1]);
  const unit = (match[2] || 'px').toLowerCase();
  if (unit === 'ex') return number * MATH_EX_PT;
  if (unit === 'pt') return number;
  return number * 0.75;
}

function setRootDimension(svg, name, value) {
  const tag = svg.match(/<svg\b[^>]*>/i)?.[0];
  if (!tag) throw new Error('renderer returned text without an SVG root');
  const pattern = new RegExp(`\\s${name}="[^"]*"`);
  const replacement = ` ${name}="${value}"`;
  const nextTag = pattern.test(tag) ? tag.replace(pattern, replacement) : tag.replace(/>$/, `${replacement}>`);
  return svg.replace(tag, nextTag);
}

function normalizeSvgDimensions(svg, kind) {
  const match = svg.match(/<svg\b[^>]*>/i);
  if (!match) throw new Error('renderer returned text without an SVG root');
  let tag = match[0];
  let svgWidth = attr(tag, 'width');
  let svgHeight = attr(tag, 'height');
  const viewBox = attr(tag, 'viewBox');
  const view = parseViewBox(viewBox);
  if (kind === 'math') {
    const widthPt = lengthToPt(svgWidth);
    const heightPt = lengthToPt(svgHeight);
    if (widthPt === null || heightPt === null) {
      throw new Error(`MathJax SVG dimensions must be measurable (width=${svgWidth}, height=${svgHeight})`);
    }
    svgWidth = `${widthPt.toFixed(3)}pt`;
    svgHeight = `${heightPt.toFixed(3)}pt`;
    svg = setRootDimension(svg, 'width', svgWidth);
    svg = setRootDimension(svg, 'height', svgHeight);
  } else if ((!lengthToPt(svgWidth) || !lengthToPt(svgHeight)) && view) {
    // Mermaid commonly emits width="100%" with a viewBox but no height.
    // Use the viewBox coordinate extent as intrinsic CSS-pixel dimensions.
    svgWidth = `${view.width}px`;
    svgHeight = `${view.height}px`;
    svg = setRootDimension(svg, 'width', svgWidth);
    svg = setRootDimension(svg, 'height', svgHeight);
  }
  const widthPt = lengthToPt(svgWidth);
  const heightPt = lengthToPt(svgHeight);
  if (widthPt === null || heightPt === null) throw new Error('SVG has no measurable width/height or viewBox');
  return {
    svg,
    width: svgWidth,
    height: svgHeight,
    viewBox,
    widthPt: Number(widthPt.toFixed(3)),
    heightPt: Number(heightPt.toFixed(3)),
    overWidthLimit: widthPt > MAX_LAYOUT_WIDTH_PT,
    overHeightLimit: heightPt > MAX_LAYOUT_HEIGHT_PT,
  };
}

async function writeAtomic(outfile, contents) {
  await fs.mkdir(path.dirname(outfile), { recursive: true });
  const temp = `${outfile}.tmp-${process.pid}-${crypto.randomBytes(5).toString('hex')}`;
  await fs.writeFile(temp, contents, 'utf8');
  await fs.rename(temp, outfile);
}

async function readJsonMaybe(file) {
  try {
    return JSON.parse(await fs.readFile(file, 'utf8'));
  } catch (error) {
    if (error.code === 'ENOENT') return null;
    return null;
  }
}

async function openMathPage(browser) {
  const page = await browser.newPage({ viewport: { width: 1200, height: 900 } });
  page.setDefaultTimeout(DEFAULT_TIMEOUT_MS);
  await page.goto('about:blank');
  await page.evaluate(() => {
    window.MathJax = {
      startup: { typeset: false },
      tex: { packages: { '[+]': ['ams'] } },
      svg: { fontCache: 'none' },
      options: { enableMenu: false },
    };
  });
  await page.addScriptTag({
    url: `https://cdn.jsdelivr.net/npm/mathjax@${MATHJAX_VERSION}/es5/tex-svg.js`,
  });
  await page.waitForFunction(() => Boolean(window.MathJax?.startup?.promise));
  await page.evaluate(() => window.MathJax.startup.promise);
  return page;
}

async function renderMath(page, asset) {
  const renderTex = typeof asset.render_tex === 'string' ? asset.render_tex : asset.tex;
  const result = await page.evaluate(async ({ tex, originalTex, display, annotateOriginalTex }) => {
    const container = await MathJax.tex2svgPromise(tex, { display });
    const svg = container.querySelector('svg');
    if (!svg) throw new Error('MathJax produced no SVG element');
    const errors = [...container.querySelectorAll('mjx-merror, merror')]
      .map((el) => (el.textContent || '').trim())
      .filter(Boolean);
    // MathJax 3.2 may serialize a TeX parse error as red mtext paths rather
    // than retaining an merror node inside the detached SVG container.
    const redMtext = Boolean(container.querySelector(
      'g[data-mml-node="mtext"][fill="red"], g[data-mml-node="mtext"][stroke="red"]',
    ));
    let mathml = await MathJax.tex2mmlPromise(tex, { display });
    if (annotateOriginalTex) {
      const parsed = new DOMParser().parseFromString(mathml, 'application/xml');
      const math = parsed.documentElement;
      if (!math || math.localName !== 'math' || parsed.querySelector('parsererror')) {
        throw new Error('MathJax MathML could not be parsed before preserving original TeX annotation');
      }
      const ns = math.namespaceURI || 'http://www.w3.org/1998/Math/MathML';
      let semantics = [...math.children].find((el) => el.localName === 'semantics');
      if (!semantics) {
        semantics = parsed.createElementNS(ns, 'semantics');
        while (math.firstChild) semantics.appendChild(math.firstChild);
        math.appendChild(semantics);
      }
      const annotation = parsed.createElementNS(ns, 'annotation');
      annotation.setAttribute('encoding', 'application/x-tex');
      annotation.textContent = originalTex;
      semantics.appendChild(annotation);
      mathml = new XMLSerializer().serializeToString(math);
    }
    return { svg: svg.outerHTML, mathml, errors, redMtext };
  }, { tex: renderTex, originalTex: asset.tex, display: asset.display === true, annotateOriginalTex: typeof asset.render_tex === 'string' });
  if (result.errors.length || result.redMtext || /<mjx-merror\b|Math input error/i.test(result.svg)) {
    throw new Error(`MathJax reported visible error markup: ${result.errors.join(' | ') || (result.redMtext ? 'red mtext parse error' : 'Math input error')}`);
  }
  if (!/^\s*<math\b[\s\S]*<\/math>\s*$/.test(result.mathml) || /<merror\b/i.test(result.mathml)) {
    throw new Error('MathJax produced missing, malformed, or error-marked MathML');
  }
  return { svg: result.svg, mathml: result.mathml };
}

function mathmlPathFor(svgPath) {
  return svgPath.replace(/\.svg$/i, '.mml');
}

async function openMermaidPage(browser) {
  const page = await browser.newPage({ viewport: { width: 1600, height: 1100 } });
  page.setDefaultTimeout(DEFAULT_TIMEOUT_MS);
  await page.goto('about:blank');
  await page.addScriptTag({
    url: `https://cdn.jsdelivr.net/npm/mermaid@${MERMAID_VERSION}/dist/mermaid.min.js`,
  });
  await page.evaluate(() => mermaid.initialize({
    startOnLoad: false,
    securityLevel: 'strict',
    theme: 'neutral',
    htmlLabels: false,
  }));
  return page;
}

async function renderMermaid(page, asset) {
  const renderCode = typeof asset.render_code === 'string' ? asset.render_code : asset.code;
  const result = await page.evaluate(async ({ id, code }) => {
    const rendered = await mermaid.render(id, code);
    const holder = document.createElement('div');
    holder.innerHTML = rendered.svg;
    const svg = holder.querySelector('svg');
    if (!svg) throw new Error('Mermaid produced no SVG element');
    return svg.outerHTML;
  }, { id: `diagram-${sha256(asset.id).slice(0, 16)}`, code: renderCode });
  return result;
}

async function runPool(items, limit, worker) {
  let cursor = 0;
  const count = Math.min(limit, items.length);
  await Promise.all(Array.from({ length: count }, async () => {
    while (true) {
      const index = cursor++;
      if (index >= items.length) return;
      await worker(items[index]);
    }
  }));
}

async function main() {
  const jobsFile = process.argv[2];
  const reportArgument = process.argv[3];
  if (!jobsFile) {
    console.error('Usage: node render_assets.cjs jobs.json [report.json]');
    process.exitCode = 2;
    return;
  }

  const jobsPath = path.resolve(jobsFile);
  const jobsDir = path.dirname(jobsPath);
  const reportPath = path.resolve(reportArgument || path.join(jobsDir, 'render_assets.report.json'));
  const loaded = JSON.parse(await fs.readFile(jobsPath, 'utf8'));
  const jobs = Array.isArray(loaded) ? loaded : loaded.assets;
  if (!Array.isArray(jobs)) throw new Error('jobs.json must be an array or an object with an assets array');

  const unique = new Map();
  const inputErrors = [];
  jobs.forEach((asset, index) => {
    try {
      assertJob(asset, index);
      const contentHash = stableAssetHash(asset);
      const previous = unique.get(asset.id);
      if (previous && previous.contentHash !== contentHash) {
        throw new Error(`duplicate id '${asset.id}' has conflicting input`);
      }
      if (!previous) unique.set(asset.id, { asset, contentHash });
    } catch (error) {
      inputErrors.push({ index, id: asset?.id ?? null, error: String(error.message || error) });
    }
  });

  const priorReport = await readJsonMaybe(reportPath);
  const priorById = new Map((priorReport?.assets || []).map((entry) => [entry.id, entry]));
  const assets = [...unique.values()].map(({ asset, contentHash }) => ({
    ...asset,
    contentHash,
    outfileResolved: path.resolve(jobsDir, asset.outfile),
  }));
  const results = [];
  const pending = [];

  for (const asset of assets) {
    const old = priorById.get(asset.id);
    let skip = false;
    try {
      if (old && (old.status === 'rendered' || old.status === 'skipped_validated') &&
          old.input_sha256 === asset.contentHash &&
          old.renderer_version === RENDERER_VERSION && old.outfile === asset.outfileResolved) {
        const svg = await fs.readFile(asset.outfileResolved, 'utf8');
        const svgHash = sha256(svg);
        const dims = normalizeSvgDimensions(svg, asset.kind);
        skip = svgHash === old.svg_sha256 && dims.width === old.svg_width &&
          dims.height === old.svg_height && dims.viewBox === old.viewBox;
        if (skip && asset.kind === 'math') {
          const mmlPath = mathmlPathFor(asset.outfileResolved);
          const mathml = await fs.readFile(mmlPath, 'utf8');
          skip = old.mathml_outfile === mmlPath && sha256(mathml) === old.mathml_sha256 &&
            /^\s*<math\b[\s\S]*<\/math>\s*$/.test(mathml) && !/<merror\b/i.test(mathml);
        }
        if (skip) results.push({ ...old, status: 'skipped_validated' });
      }
    } catch (_) {
      skip = false;
    }
    if (!skip) pending.push(asset);
  }

  let browser = null;
  const errors = [...inputErrors];
  try {
    if (pending.length) {
      browser = await chromium.launch({ executablePath: CHROME, headless: true });
      const mathJobs = pending.filter((asset) => asset.kind === 'math');
      const mermaidJobs = pending.filter((asset) => asset.kind === 'mermaid');

      const mathPages = [];
      try {
        for (let i = 0; i < Math.min(MAX_MATH_PAGES, mathJobs.length); i++) {
          mathPages.push(await openMathPage(browser));
        }
      } catch (error) {
        for (const page of mathPages) await page.close().catch(() => {});
        for (const asset of mathJobs) errors.push({ id: asset.id, kind: asset.kind, outfile: asset.outfileResolved, input_sha256: asset.contentHash, error: `MathJax page setup failed: ${String(error.stack || error)}` });
      }

      if (mathPages.length) {
        await runPool(mathJobs, mathPages.length, async (asset) => {
          const page = mathPages.shift();
          try {
            const rendered = await renderMath(page, asset);
            const dims = normalizeSvgDimensions(rendered.svg, asset.kind);
            await writeAtomic(asset.outfileResolved, dims.svg || rendered.svg);
            const finalSvg = dims.svg || rendered.svg;
            const mathmlPath = mathmlPathFor(asset.outfileResolved);
            await writeAtomic(mathmlPath, rendered.mathml);
            const mathResult = { id: asset.id, kind: 'math', status: 'rendered', outfile: asset.outfileResolved, mathml_outfile: mathmlPath, input_sha256: asset.contentHash, tex_sha256: sha256(asset.tex), original_tex_sha256: sha256(asset.tex), render_tex_sha256: sha256(typeof asset.render_tex === 'string' ? asset.render_tex : asset.tex), mathml_sha256: sha256(rendered.mathml), renderer_version: RENDERER_VERSION, svg_sha256: sha256(finalSvg), svg_width: dims.width, svg_height: dims.height, true_width_pt: dims.widthPt, true_height_pt: dims.heightPt, viewBox: dims.viewBox, over_width_396pt: dims.overWidthLimit, over_height_500pt: dims.overHeightLimit, display: asset.display === true };
            if (typeof asset.render_tex === 'string') {
              mathResult.normalization_protocol = asset.normalization_protocol || 'render-normalization-v1';
              mathResult.normalization_reason = asset.normalization_reason || null;
              mathResult.normalization_ref = asset.normalization_ref || null;
              mathResult.mathml_original_tex_annotation = true;
            }
            results.push(mathResult);
          } catch (error) {
            errors.push({ id: asset.id, kind: asset.kind, outfile: asset.outfileResolved, input_sha256: asset.contentHash, tex_sha256: sha256(asset.tex), original_tex_sha256: sha256(asset.tex), render_tex_sha256: sha256(typeof asset.render_tex === 'string' ? asset.render_tex : asset.tex), normalization_ref: asset.normalization_ref || null, error: String(error.stack || error) });
          } finally {
            mathPages.push(page);
          }
        });
        await Promise.all(mathPages.map((page) => page.close().catch(() => {})));
      }

      let mermaidPage = null;
      if (mermaidJobs.length) {
        try {
          mermaidPage = await openMermaidPage(browser);
        } catch (error) {
          for (const asset of mermaidJobs) errors.push({ id: asset.id, kind: asset.kind, outfile: asset.outfileResolved, input_sha256: asset.contentHash, error: `Mermaid page setup failed: ${String(error.stack || error)}` });
        }
      }
      if (mermaidPage) {
        for (const asset of mermaidJobs) {
          try {
            const svg = await renderMermaid(mermaidPage, asset);
            const dims = normalizeSvgDimensions(svg, asset.kind);
            await writeAtomic(asset.outfileResolved, dims.svg || svg);
            const finalSvg = dims.svg || svg;
            const diagramResult = { id: asset.id, kind: 'mermaid', status: 'rendered', outfile: asset.outfileResolved, input_sha256: asset.contentHash, mermaid_source_sha256: sha256(asset.code), original_code_sha256: sha256(asset.code), render_code_sha256: sha256(typeof asset.render_code === 'string' ? asset.render_code : asset.code), renderer_version: RENDERER_VERSION, svg_sha256: sha256(finalSvg), svg_width: dims.width, svg_height: dims.height, true_width_pt: dims.widthPt, true_height_pt: dims.heightPt, viewBox: dims.viewBox, over_width_396pt: dims.overWidthLimit, over_height_500pt: dims.overHeightLimit };
            if (typeof asset.render_code === 'string') {
              diagramResult.normalization_protocol = asset.normalization_protocol || 'render-normalization-v1';
              diagramResult.normalization_reason = asset.normalization_reason || null;
              diagramResult.normalization_ref = asset.normalization_ref || null;
            }
            results.push(diagramResult);
          } catch (error) {
            errors.push({ id: asset.id, kind: asset.kind, outfile: asset.outfileResolved, input_sha256: asset.contentHash, mermaid_source_sha256: sha256(asset.code), original_code_sha256: sha256(asset.code), render_code_sha256: sha256(typeof asset.render_code === 'string' ? asset.render_code : asset.code), normalization_ref: asset.normalization_ref || null, error: String(error.stack || error) });
          }
        }
        await mermaidPage.close().catch(() => {});
      }
    }
  } catch (error) {
    for (const asset of pending) {
      const accounted = results.some((item) => item.id === asset.id) || errors.some((item) => item.id === asset.id);
      if (!accounted) errors.push({ id: asset.id, kind: asset.kind, outfile: asset.outfileResolved, input_sha256: asset.contentHash, error: `Browser launch/render setup failed: ${String(error.stack || error)}` });
    }
  } finally {
    if (browser) await browser.close().catch(() => {});
  }

  results.sort((a, b) => a.id.localeCompare(b.id));
  errors.sort((a, b) => String(a.id ?? a.index).localeCompare(String(b.id ?? b.index)));
  const report = {
    schema: 'render-assets-report-v1',
    renderer_version: RENDERER_VERSION,
    usage_note: 'SVGs are standalone native-layout assets. EPUB equations must retain accessible MathML; do not replace equation content with SVG alone.',
    jobs_file: jobsPath,
    generated_at: new Date().toISOString(),
    input_count: jobs.length,
    unique_asset_count: unique.size,
    rendered: results.filter((item) => item.status === 'rendered').length,
    skipped_validated: results.filter((item) => item.status === 'skipped_validated').length,
    failed: errors.length,
    assets: results,
    errors,
  };
  await writeAtomic(reportPath, `${JSON.stringify(report, null, 2)}\n`);
  console.log(JSON.stringify({ report: reportPath, input_count: report.input_count, unique_asset_count: report.unique_asset_count, rendered: report.rendered, skipped_validated: report.skipped_validated, failed: report.failed }));
  if (errors.length) process.exitCode = 1;
}

main().catch((error) => {
  console.error(String(error.stack || error));
  process.exitCode = 1;
});
