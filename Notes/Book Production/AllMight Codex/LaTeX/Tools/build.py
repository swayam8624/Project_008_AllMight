#!/usr/bin/env python3
"""Compile seven learning stages or their page-limited physical books."""
import argparse
import concurrent.futures
import hashlib
import json
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from pypdf import PdfReader

from book02_visuals import install_book02_visuals

ROOT = Path(__file__).resolve().parent.parent
BUILD_ROOT = ROOT / 'Build'
TARGET_PAGES = 650
MAX_PAGES = 800


def read(path):
    return json.loads(Path(path).read_text())


def write_json(path, value):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + '.tmp')
    tmp.write_text(json.dumps(value, indent=2, ensure_ascii=False) + '\n')
    tmp.replace(path)


def generation_path(value=None):
    if value:
        path = Path(value)
        if not path.is_absolute():
            path = (ROOT / path).resolve()
    else:
        state = read(ROOT / 'CURRENT.json')
        path = (ROOT / state['generation_path']).resolve()
    if not path.is_dir() or not path.resolve().is_relative_to(ROOT.resolve()):
        raise SystemExit('Invalid generation path')
    return path


def tex_runtime():
    candidate = Path('/usr/local/texlive/2025/bin/universal-darwin/latexmk')
    engine = os.environ.get('ALLMIGHT_LATEXMK') or (str(candidate) if candidate.exists() else shutil.which('latexmk'))
    if not engine:
        raise SystemExit('latexmk not found; set ALLMIGHT_LATEXMK or install the pinned TeX Live 2025 runtime')
    # Keep the caller's pinned TeX Live bin path; resolving its compatibility symlink can select another release.
    engine = str(Path(engine).absolute())
    env = os.environ.copy()
    env['PATH'] = str(Path(engine).parent) + os.pathsep + env.get('PATH', '')
    env['TEXINPUTS'] = env.get('TEXINPUTS', '')
    env.setdefault('TEXMFHOME', '/tmp/allmight-local-texmf')
    return engine, env


def prepare_book_font_assets(cwd):
    """Stage the repository's existing licensed font in this TeX working copy.

    allmight-book.sty intentionally searches `Assets/Fonts/` relative to
    latexmk's working directory. The nested Book Two candidate directory is
    deeper than that stylesheet's historical relative-path guesses. Copy the
    *existing* source asset into the isolated build working directory; never
    move it out of the canonical Master repository or alter original chapters.
    """
    cwd = Path(cwd)
    source = ROOT / 'Assets' / 'Fonts' / 'NotoEmoji-Static.ttf'
    if not source.is_file() or source.stat().st_size < 100_000:
        raise RuntimeError(
            'The repository font asset is missing or incomplete: ' + str(source) +
            '. Restore the tracked asset in LaTeX/Assets/Fonts before compiling.'
        )
    target_dir = cwd / 'Assets' / 'Fonts'
    target_dir.mkdir(parents=True, exist_ok=True)
    target = target_dir / source.name
    # The working copy can be rebuilt repeatedly. Only touch the nested
    # temporary asset, never the original font under ROOT/Assets/Fonts.
    if not target.is_file() or target.stat().st_size != source.stat().st_size:
        shutil.copy2(source, target)
    if not target.is_file() or target.stat().st_size != source.stat().st_size:
        raise RuntimeError('Could not stage Noto Emoji at ' + str(target))
    return target


def compile_tex(cwd, engine, env, generation=None):
    cwd = Path(cwd)
    env = env.copy()
    env['TEXINPUTS'] = str(cwd.resolve()) + os.pathsep + env.get('TEXINPUTS', '')
    shutil.copy2(ROOT / 'allmight-book.sty', cwd / 'allmight-book.sty')
    prepare_book_font_assets(cwd)
    (cwd / 'build').mkdir(parents=True, exist_ok=True)
    command = [engine, '-g', '-lualatex', '-interaction=nonstopmode', '-halt-on-error',
               '-file-line-error', '-outdir=build', 'main.tex']
    log = cwd / 'build' / 'build-command.log'
    with log.open('w') as stream:
        result = subprocess.run(command, cwd=cwd, env=env, stdout=stream, stderr=subprocess.STDOUT)
    pdf = cwd / 'build' / 'main.pdf'
    if result.returncode == 0 and generation is not None:
        fls = cwd / 'build' / 'main.fls'
        if not fls.is_file():
            raise RuntimeError('TeX recorder output is missing: ' + str(fls))
        inputs = [Path(line[6:]) for line in fls.read_text(errors='replace').splitlines() if line.startswith('INPUT ')]
        resolved = {(p if p.is_absolute() else cwd / p).resolve() for p in inputs}
        expected = {generation.resolve() / x for x in re.findall(r'\\input\{\.\./current/([^}]+)\}', (cwd / 'main.tex').read_text())}
        if not expected or not expected.issubset(resolved):
            raise RuntimeError('TeX did not read all requested generation fragments: ' + str(sorted(map(str, expected - resolved))[:5]))
        wrong = [str(p) for p in inputs if '/current/' in str(p) and not (p if p.is_absolute() else cwd / p).resolve().is_relative_to(generation.resolve())]
        if wrong:
            raise RuntimeError('TeX read another generation through current: ' + str(wrong[:5]))
    if result.returncode == 0 and pdf.is_file():
        shutil.copy2(pdf, cwd / 'main.pdf')
    return {'command': command, 'returncode': result.returncode, 'pdf': str(pdf),
            'compile_log_path': str(log), 'pages': len(PdfReader(str(pdf)).pages) if result.returncode == 0 and pdf.is_file() else None}


def read_aux_labels(path):
    labels = {}
    if not Path(path).is_file():
        return labels
    pattern = re.compile(r'\\newlabel\{([^}]+)\}\{\{[^{}]*\}\{(\d+)\}')
    for line in Path(path).read_text(errors='replace').splitlines():
        match = pattern.search(line)
        if match:
            labels[match.group(1)] = int(match.group(2))
    return labels


def page_weights(generation, stage, stage_rows, stage_attachments, labels, total_pages):
    chapter_ids = stage['chapters']
    chapter_rows = {r['id']: r for r in stage_rows if r['kind'] == 'chapter'}
    missing = [cid for cid in chapter_ids if 'stagechapter:' + cid not in labels]
    if missing:
        raise RuntimeError('Missing chapter page labels for ' + ', '.join(missing[:5]))
    chapter_end = labels.get('stagechaptersend:' + stage['id'])
    if chapter_end is None:
        raise RuntimeError('Missing chapter-end page label for ' + stage['id'])
    chapter_pages = {}
    starts = [labels['stagechapter:' + cid] for cid in chapter_ids]
    for index, cid in enumerate(chapter_ids):
        start = starts[index]
        end = starts[index + 1] - 1 if index + 1 < len(starts) else chapter_end
        chapter_pages[cid] = max(1, end - start + 1)

    reference_rows = [r for r in stage_rows if r['kind'] == 'reference']
    reference_pages = {}
    ref_end = labels.get('stagereferencesend:' + stage['id'], chapter_end)
    for index, row in enumerate(reference_rows):
        key = 'referencestart:' + row['id']
        if key not in labels:
            raise RuntimeError('Missing reference page label for ' + row['id'])
        start = labels[key]
        next_start = None
        for later in reference_rows[index + 1:]:
            label = 'referencestart:' + later['id']
            if label in labels:
                next_start = labels[label]
                break
        end = next_start - 1 if next_start is not None else ref_end
        reference_pages[row['id']] = max(0, end - start + 1)

    attachment_pages = {}
    attachments = list(stage_attachments.values())
    for index, attachment in enumerate(attachments):
        key = 'attachmentstart:' + attachment['id']
        if key not in labels:
            raise RuntimeError('Missing attachment page label for ' + attachment['id'])
        start = labels[key]
        next_start = None
        for later in attachments[index + 1:]:
            label = 'attachmentstart:' + later['id']
            if label in labels:
                next_start = labels[label]
                break
        end = next_start - 1 if next_start is not None else total_pages
        attachment_pages[attachment['id']] = max(0, end - start + 1)

    return {'chapters': chapter_pages, 'references': reference_pages,
            'attachments': attachment_pages, 'frontmatter_pages': max(1, starts[0] - 1),
            'total_pages': total_pages}


def companion_rows_for(stage_rows):
    refs = []
    for row in stage_rows:
        if row['kind'] != 'reference':
            continue
        refs.append({'id': row['id'], 'source': row['source'], 'source_sha256': row['source_sha256'],
                     'title': Path(row['source']).stem,
                     'first_use_chapter': row['first_use_chapter'],
                     'direct_first_use_chapter': row.get('direct_first_use_chapter'),
                     'first_use_reason': row.get('first_use_reason'), 'part_id': None,
                     'estimated_pages': 0})
    return refs


def source_attachments_for(manifest, stage):
    attachments_by_part = manifest.get('attachments', {})
    physical = manifest.get('physical_parts') or {}
    stage_map = next((s for s in physical.get('stages', []) if s['id'] == stage['id']), None)
    part_ids = [p['id'] for p in stage_map['parts']] if stage_map else [stage['id']]
    result = []
    for part_id in part_ids:
        for attachment in sorted(attachments_by_part.get(part_id, {}).values(), key=lambda a: a['name']):
            bundle = attachment.get('bundle', '')
            bundle_path = bundle.removeprefix('../current/')
            result.append({'id': attachment['id'], 'source': attachment['source'],
                           'source_sha256': attachment['sha256'], 'title': attachment['name'],
                           'bundle_path': bundle_path, 'source_paths': attachment.get('source_paths', [attachment['source']]),
                           'link_uses': attachment.get('link_uses', []),
                           'first_use_chapter': attachment.get('first_use_chapter'),
                           'part_id': part_id, 'included_via': 'VerbatimInput',
                           'estimated_pages': 0})
    return result


def plan_parts(generation, proof_root, catalog, manifest, stage_weights):
    by_stage_rows = {s['id']: [r for r in manifest['rows'] if r['stage_id'] == s['id']] for s in catalog['stages']}
    stages_out = []
    for stage in catalog['stages']:
        rows = by_stage_rows[stage['id']]
        weights = stage_weights[stage['id']]
        refs = [r for r in rows if r['kind'] == 'reference']
        attachments = source_attachments_for(manifest, stage)
        note_pages = {}
        for row in refs:
            note_pages[row['first_use_chapter']] = note_pages.get(row['first_use_chapter'], 0) + weights['references'][row['id']]
        for attachment in attachments:
            owner = attachment.get('first_use_chapter')
            if owner:
                note_pages[owner] = note_pages.get(owner, 0) + weights['attachments'][attachment['id']]
        chapter_id_by_source = {str(Path(row['source']).resolve()): row['id']
                                for row in rows if row['kind'] == 'chapter'}
        navigation_rows = {}
        for navigation in manifest.get('chapter_navigation', {}).get('records', []):
            # Converter records are source-qualified; older records may carry an id directly.
            chapter_id = navigation.get('chapter_id') or chapter_id_by_source.get(
                str(Path(navigation.get('source', '')).resolve()))
            if chapter_id:
                navigation_rows[chapter_id] = navigation
        chapter_page_rows = []
        for cid in stage['chapters']:
            source_row = next(r for r in rows if r['kind'] == 'chapter' and r['id'] == cid)
            own_weight = weights['chapters'][cid]
            nav = navigation_rows.get(cid, {})
            chapter_page_rows.append({'id': cid, 'title': Path(source_row['source']).stem,
                                      'source': source_row['source'],
                                      'logical_chapter_number': nav.get('logical_chapter_number'),
                                      'previous_chapter_id': nav.get('previous_chapter_id'),
                                      'next_chapter_id': nav.get('next_chapter_id'),
                                      'previous_target': nav.get('previous_target'), 'next_target': nav.get('next_target'),
                                      'estimated_pages': own_weight + note_pages.get(cid, 0)})
        # First find the fewest contiguous chapter-boundary parts that fit under the hard cap.
        # Then minimize squared page loads for that part count to avoid an unnecessary thin final part.
        front = weights['frontmatter_pages']
        chapter_weights = [c['estimated_pages'] for c in chapter_page_rows]
        for chapter, pages in zip(chapter_page_rows, chapter_weights):
            if front + pages > MAX_PAGES:
                raise RuntimeError('One chapter and its first-use references exceed physical maximum: ' + chapter['id'])
        minimal_parts = 1; running = front
        for pages in chapter_weights:
            if running + pages > MAX_PAGES:
                minimal_parts += 1; running = front
            running += pages
        n = len(chapter_page_rows)
        prefix = [0]
        for pages in chapter_weights: prefix.append(prefix[-1] + pages)
        infinity = float('inf')
        dp = [[infinity] * (n + 1) for _ in range(minimal_parts + 1)]
        previous = [[None] * (n + 1) for _ in range(minimal_parts + 1)]
        dp[0][0] = 0
        for groups_used in range(1, minimal_parts + 1):
            for end in range(groups_used, n + 1):
                for start in range(groups_used - 1, end):
                    load = prefix[end] - prefix[start] + front
                    if load > MAX_PAGES or dp[groups_used - 1][start] == infinity: continue
                    cost = dp[groups_used - 1][start] + load * load
                    if cost < dp[groups_used][end]:
                        dp[groups_used][end] = cost; previous[groups_used][end] = start
        boundaries = []; end = n
        for groups_used in range(minimal_parts, 0, -1):
            start = previous[groups_used][end]
            if start is None: raise RuntimeError('Could not balance chapter-boundary parts for ' + stage['id'])
            boundaries.append((start, end)); end = start
        boundaries.reverse()
        groups = []
        for start, end in boundaries:
            group = chapter_page_rows[start:end]
            groups.append((group, sum(c['estimated_pages'] for c in group) + front))
        stage_parts = []; chapter_to_part = {}
        refs_out = companion_rows_for(rows)
        for index, (chapters, estimated) in enumerate(groups, 1):
            part_id = stage['id'] + '-P' + str(index).zfill(2)
            # Reader-facing part labels use the actual chapter titles and sequence within this stage.
            part_title = stage['title'] + ' — Part ' + str(index)
            part = {'id': part_id, 'index': index, 'title': part_title,
                    'chapter_ids': [c['id'] for c in chapters], 'companion_ids': [], 'attachment_ids': [],
                    'estimated_pages': estimated, 'reading_map_destination': 'volume-reading-map',
                    'pdf_path': str(proof_root / 'physical-books' / part_id / 'main.pdf'),
                    'build_pdf_path': str(proof_root / 'physical-books' / part_id / 'build' / 'main.pdf'),
                    'tex_path': str(generation / part_id / 'main.tex'),
                    'compile_log_path': str(proof_root / 'physical-books' / part_id / 'build' / 'build-command.log'),
                    'compile_status': 'pending', 'pages': None}
            for chapter in chapters:
                chapter_to_part[chapter['id']] = part_id
            stage_parts.append(part)
        for ref in refs_out:
            home = ref.get('first_use_chapter')
            if home not in chapter_to_part:
                raise RuntimeError('Companion first-use chapter not assigned in same stage: ' + ref['id'])
            part_id = chapter_to_part[home]
            ref['part_id'] = part_id
            ref['estimated_pages'] = weights['references'].get(ref['id'], 0)
            next(p for p in stage_parts if p['id'] == part_id)['companion_ids'].append(ref['id'])
        for attachment in attachments:
            owner = attachment.get('first_use_chapter')
            if owner not in chapter_to_part:
                raise RuntimeError('Source attachment first-use chapter is not assigned in same stage: ' + attachment['id'])
            part_id = chapter_to_part[owner]
            attachment['part_id'] = part_id
            attachment['estimated_pages'] = weights['attachments'].get(attachment['id'], 0)
            next(p for p in stage_parts if p['id'] == part_id)['attachment_ids'].append(attachment['id'])
        chapter_records = []
        for chapter in chapter_page_rows:
            record = dict(chapter); record['part_id'] = chapter_to_part[chapter['id']]
            chapter_records.append(record)
        stages_out.append({'id': stage['id'], 'number': stage['number'], 'title': stage['title'],
                           'chapters': chapter_records, 'companions': refs_out, 'attachments': attachments,
                           'parts': stage_parts, 'page_weights': weights})
    attachment_total = sum(len(stage['attachments']) for stage in stages_out)
    attachment_ids = [a['id'] for stage in stages_out for a in stage['attachments']]
    if len(attachment_ids) != len(set(attachment_ids)):
        raise RuntimeError('Source attachment assigned more than once')
    return {'schema': 'allmight-physical-parts-v1', 'generation_id': manifest['generation_id'],
            'target_pages': TARGET_PAGES, 'max_pages': MAX_PAGES,
            'chapter_navigation': manifest.get('chapter_navigation'),
            'proof_root': str(proof_root), 'companion_assignment': manifest.get('companion_assignment'),
            'source_attachment_assignment': {'count': attachment_total, 'assigned_once': True,
                                             'duplicate_ids': 0, 'missing_source_sha256': sum(1 for s in stages_out for a in s['attachments'] if not a.get('source_sha256'))},
            'stages': stages_out}


def prove_and_plan(generation, catalog, manifest, reuse_generation=None, selected_stage_ids=None):
    generation_name = manifest['generation_id']
    proof_root = BUILD_ROOT / generation_name / 'stage-proofs'
    proof_root.mkdir(parents=True, exist_ok=True)
    current = proof_root / 'current'
    if current.is_symlink() or current.exists():
        if current.is_dir() and not current.is_symlink():
            raise RuntimeError('Proof current path exists as a directory')
        current.unlink()
    current.symlink_to(generation, target_is_directory=True)
    engine, env = tex_runtime()
    progress = {'generation_id': generation_name, 'status': 'running', 'stages': [],
                'reused_stage_proofs_from': None}
    progress_path = proof_root / 'stage-proof-progress.json'
    selected_stage_ids = set(selected_stage_ids or [s['id'] for s in catalog['stages']])
    if reuse_generation:
        old_generation = generation_path(reuse_generation)
        old_manifest = read(old_generation / 'conversion-manifest.json')
        old_progress = read(BUILD_ROOT / old_manifest['generation_id'] / 'stage-proofs' / 'stage-proof-progress.json')
        old_progress_by_id = {s['id']: s for s in old_progress['stages']}
        old_rows_by_stage = {s['id']: [r for r in old_manifest['rows'] if r['stage_id'] == s['id']] for s in catalog['stages']}
        new_rows_by_stage = {s['id']: [r for r in manifest['rows'] if r['stage_id'] == s['id']] for s in catalog['stages']}
        special_names = {'Keyword Index.md', 'Dependency Map.md'}
        for stage in catalog['stages']:
            sid = stage['id']
            if sid in selected_stage_ids: continue
            old_proof = old_progress_by_id.get(sid)
            if not old_proof or not old_proof.get('page_weights'):
                raise RuntimeError('No reusable completed page proof for ' + sid)
            old_chapters = {r['id']: r['source_sha256'] for r in old_rows_by_stage[sid] if r['kind'] == 'chapter'}
            new_chapters = {r['id']: r['source_sha256'] for r in new_rows_by_stage[sid] if r['kind'] == 'chapter'}
            if old_chapters != new_chapters:
                raise RuntimeError('Cannot reuse page proof; source chapter inputs changed for ' + sid)
            old_refs = {Path(r['source']).name: r for r in old_rows_by_stage[sid] if r['kind'] == 'reference'}
            new_refs = {Path(r['source']).name: r for r in new_rows_by_stage[sid] if r['kind'] == 'reference'}
            if {k:v['source_sha256'] for k,v in old_refs.items() if k not in special_names} != {k:v['source_sha256'] for k,v in new_refs.items() if k not in special_names}:
                raise RuntimeError('Cannot reuse page proof; companion assignment changed beyond the two stage-7 navigation notes for ' + sid)
            reused = json.loads(json.dumps(old_proof))
            reused['reused_from_generation'] = old_manifest['generation_id']
            if sid == '01-Bits-Programs-and-Reliable-Systems':
                for name in special_names:
                    row = old_refs.get(name)
                    if not row: continue
                    ref_id = row['id']; pages = reused['page_weights']['references'].pop(ref_id, 0)
                    home = row.get('first_use_chapter')
                    if home in reused['page_weights']['chapters']:
                        reused['page_weights']['chapters'][home] = max(1, reused['page_weights']['chapters'][home] - pages)
                    reused['page_weights']['total_pages'] = max(1, reused['page_weights']['total_pages'] - pages)
                reused['reused_note'] = 'Stage-7 reference-atlas notes were subtracted from prior stage-1 reference weights.'
            progress['stages'].append(reused)
        progress['reused_stage_proofs_from'] = old_manifest['generation_id']
    for stage in catalog['stages']:
        if stage['id'] not in selected_stage_ids: continue
        out = proof_root / stage['id']; out.mkdir(parents=True, exist_ok=True)
        proof_entry = generation / '_stage-proofs' / stage['id'] / 'main.tex'
        if not proof_entry.is_file():
            raise RuntimeError('Missing complete logical-stage proof entry: ' + str(proof_entry))
        shutil.copy2(proof_entry, out / 'main.tex')
        install_book02_visuals(out / 'main.tex', include_rich_atlas=True)
        result = compile_tex(out, engine, env)
        if result['returncode'] != 0:
            progress['status'] = 'failed'; progress['stages'].append({'id': stage['id'], **result})
            write_json(progress_path, progress)
            raise RuntimeError('Stage proof failed: ' + stage['id'] + ' (see ' + result['compile_log_path'] + ')')
        labels = read_aux_labels(out / 'build' / 'main.aux')
        rows = [r for r in manifest['rows'] if r['stage_id'] == stage['id']]
        stage_attachments = source_attachments_for(manifest, stage)
        stage_attachments = {a['id']: {'id': a['id'], 'source': a['source'], 'name': a['title'],
                                       'sha256': a['source_sha256'], 'first_use_chapter': a['first_use_chapter']}
                             for a in stage_attachments}
        weights = page_weights(generation, stage, rows, stage_attachments,
                               labels, result['pages'])
        progress['stages'].append({'id': stage['id'], 'title': stage['title'], **result,
                                   'label_count': len(labels), 'page_weights': weights})
        write_json(progress_path, progress)
        print('PROVED', stage['id'], 'pages', result['pages'], 'chapters', len(stage['chapters']), flush=True)
    progress['stages'].sort(key=lambda s: next(i for i,x in enumerate(catalog['stages']) if x['id']==s['id']))
    if len(progress['stages']) != len(catalog['stages']):
        raise RuntimeError('Stage proof ledger incomplete: '+str(len(progress['stages']))+'/'+str(len(catalog['stages'])))
    progress['status'] = 'complete'; write_json(progress_path, progress)
    weights = {s['id']: s['page_weights'] for s in progress['stages']}
    parts = plan_parts(generation, BUILD_ROOT / generation_name, catalog, manifest, weights)
    write_json(ROOT / 'physical-parts.json', parts)
    print('PHYSICAL_PARTS', ROOT / 'physical-parts.json', flush=True)
    return parts


def build_physical_parts(generation, part_map, selected=None):
    generation_name = part_map['generation_id']
    output_root = BUILD_ROOT / generation_name / 'physical-books'
    output_root.mkdir(parents=True, exist_ok=True)
    current = output_root / 'current'
    if current.is_symlink() or current.exists():
        if current.is_dir() and not current.is_symlink():
            raise RuntimeError('Physical build current path exists as a directory')
        current.unlink()
    current.symlink_to(generation, target_is_directory=True)
    engine, env = tex_runtime()
    targets = [p for s in part_map['stages'] for p in s['parts']]
    if selected:
        unknown = set(selected) - {p['id'] for p in targets}
        if unknown: raise SystemExit('Unknown physical part id(s): ' + ', '.join(sorted(unknown)))
        targets = [p for p in targets if p['id'] in selected]
    status_path = output_root / 'build-results.json'
    status = read(status_path) if status_path.exists() else {'generation_id': generation_name, 'parts': {}}

    def run(part):
        cwd = output_root / part['id']; cwd.mkdir(parents=True, exist_ok=True)
        shutil.copy2(generation / part['id'] / 'main.tex', cwd / 'main.tex')
        install_book02_visuals(cwd / 'main.tex', include_rich_atlas=True)
        result = compile_tex(cwd, engine, env, generation)
        if result['returncode'] == 0:
            build_pdf = Path(result['pdf'])
            candidate_pdf = cwd / 'main.pdf'
            shutil.copy2(build_pdf, candidate_pdf)
            build_sha = hashlib.sha256(build_pdf.read_bytes()).hexdigest()
            candidate_sha = hashlib.sha256(candidate_pdf.read_bytes()).hexdigest()
            if build_sha != candidate_sha:
                raise RuntimeError('Copied candidate PDF hash mismatch for ' + part['id'])
            result.update({'build_pdf_path': str(build_pdf), 'candidate_pdf_path': str(candidate_pdf),
                           'build_pdf_sha256': build_sha, 'candidate_pdf_sha256': candidate_sha})
        return part['id'], result

    with concurrent.futures.ThreadPoolExecutor(max_workers=2) as pool:
        futures = [pool.submit(run, p) for p in targets]
        for future in concurrent.futures.as_completed(futures):
            part_id, result = future.result()
            status['parts'][part_id] = result
            write_json(status_path, status)
            match = next(p for p in targets if p['id'] == part_id)
            candidate_pdf = output_root / part_id / 'main.pdf'
            build_pdf = output_root / part_id / 'build' / 'main.pdf'
            match.update({'compile_status': 'passed' if result['returncode'] == 0 else 'failed',
                          'pages': result['pages'], 'pdf_path': str(candidate_pdf),
                          'build_pdf_path': str(build_pdf),
                          'pdf_sha256': result.get('candidate_pdf_sha256'),
                          'build_pdf_sha256': result.get('build_pdf_sha256'),
                          'compile_log_path': result['compile_log_path']})
            print('BUILT', part_id, 'returncode', result['returncode'], 'pages', result['pages'], flush=True)
    write_json(ROOT / 'physical-parts.json', part_map)
    failures = [p for p in targets if p.get('compile_status') != 'passed']
    over_limit = [p for p in targets if p.get('pages') and p['pages'] > part_map['max_pages']]
    if failures or over_limit:
        if over_limit:
            print('OVER_LIMIT', json.dumps([{'id': p['id'], 'pages': p['pages']} for p in over_limit]), flush=True)
        raise SystemExit(1)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--generation', help='staged generation path or id; defaults to CURRENT.json')
    parser.add_argument('--plan-physical-parts', action='store_true', help='compile seven full stages and write page-weighted parts')
    parser.add_argument('--build-physical-parts', action='store_true', help='compile mapped physical books, with max two concurrent jobs')
    parser.add_argument('--reuse-stage-proofs-from', help='staged generation with unchanged chapters and prior page-weight proofs')
    parser.add_argument('--only-stage', action='append', help='stage id to compile when reusing other stage proofs')
    parser.add_argument('parts', nargs='*', help='optional physical part ids to build')
    args = parser.parse_args()
    generation = generation_path(args.generation)
    if args.plan_physical_parts:
        manifest = read(generation / 'conversion-manifest.json')
        catalog = read(ROOT / 'Tools' / 'stage-catalog.json')
        prove_and_plan(generation, catalog, manifest, args.reuse_stage_proofs_from, args.only_stage)
    elif args.build_physical_parts:
        part_map = read(ROOT / 'physical-parts.json')
        if part_map['generation_id'] != read(generation / 'conversion-manifest.json')['generation_id']:
            raise SystemExit('Physical part map does not match generation; rerun --plan-physical-parts')
        build_physical_parts(generation, part_map, args.parts)
    else:
        raise SystemExit('Choose --plan-physical-parts or --build-physical-parts')


if __name__ == '__main__':
    main()
