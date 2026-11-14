#!/usr/bin/env python3
"""Build/check a source-preserving chapter edition; never edit its three masters."""
import argparse
from bisect import bisect_right
import hashlib
import json
import os
import math
from urllib.parse import unquote, quote
from pathlib import Path
import re
from note_integrity import INLINE_OR_WIKI, code_blocks, prose_lines
from check_vault_links import heading_key, key

ROOT = Path(__file__).resolve().parents[1]
EDITION = 'Chapter Edition'
BODY_START = '<!-- CHAPTER-SOURCE-BEGIN -->\n'
BODY_END = '<!-- CHAPTER-SOURCE-END -->'


def digest(text):
    return hashlib.sha256(text.encode('utf-8')).hexdigest()


def anchors(text):
    """Recognize real headings/bold section labels, outside fences and display math."""
    visible = dict(prose_lines(text))
    result = []
    occurrences = {}
    opened = False
    for number, line in visible.items():
        if '$$' in line:
            opened = bool(opened ^ (line.count('$$') % 2))
            continue
        if opened:
            continue
        match = re.match(r'^(#{1,6})\s+(.+?)\s*$', line)
        bold = re.match(r'^\*\*([^*\n]+)\*\*[:.]?\s*$', line)
        if match or bold:
            title = match[2] if match else bold[1]
            occurrences[title] = occurrences.get(title, 0) + 1
            result.append({'line': number, 'heading': title,
                           'occurrence': occurrences[title],
                           'level': len(match[1]) if match else 5,
                           'kind': 'heading' if match else 'bold'})
    return result


def resolve_anchor(anchor, available, length):
    if anchor['heading'] == 'BOOK_START':
        return 1
    if anchor['heading'] == 'BOOK_END':
        return length + 1
    matches = [h for h in available if h['heading'] == anchor['heading']
               and h['occurrence'] == anchor['occurrence']]
    if len(matches) != 1:
        raise ValueError(f"Unresolved chapter boundary: {anchor}")
    return matches[0]['line']


def filename(number, title):
    title = re.sub(r'[\\/:*?"<>|#\[\]`]', '', title)
    title = ' '.join(title.split()).strip(' .')[:110].rstrip(' .')
    return f'{number:02d} - {title}.md'


def link(path, label):
    return '[[' + str(Path(path).with_suffix('')) + '|' + label + ']]'


def estimate_pages(text):
    blocks = code_blocks(text)
    pages = 0.0
    for block in blocks:
        pages += .5 if block.language == 'mermaid' else .03 + sum(
            max(1, math.ceil(len(line) / 95)) / 48 for line in block.body.splitlines())
    opened = False
    for _, line in prose_lines(text):
        if '$$' in line:
            pages += .025
            opened = bool(opened ^ (line.count('$$') % 2))
        elif opened:
            pages += max(1, math.ceil(len(line.rstrip()) / 95)) / 36
        elif re.match(r'^#{1,6}\s', line):
            pages += .045 if line.startswith(('# ', '## ')) else .03
        elif re.match(r'^\*\*[^*]+\*\*[:.]?\s*$', line):
            pages += .03
        elif re.search(r'!\[\[|!\[[^\]]*\]\(', line):
            pages += .45
        elif line.lstrip().startswith('|'):
            pages += 1 / 36
        elif not line.strip():
            pages += .003
        else:
            pages += len(line.split()) / 400
    return round(pages, 1)


def prepare(root, plans):
    books = []
    for plan in plans:
        text = (root / plan['source_book']).read_text(encoding='utf-8')
        lines = text.splitlines(keepends=True)
        heads = anchors(text)
        chapters = []
        for c in plan['chapters']:
            start = resolve_anchor(c['start_anchor'], heads, len(lines))
            end = resolve_anchor(c['end_anchor'], heads, len(lines))
            if end <= start:
                raise ValueError('Chapter boundaries must increase')
            for field in ('title', 'entry_bridge', 'learning_goal', 'recall_question'):
                if not c.get(field, '').strip():
                    raise ValueError(f"Missing {field} in chapter {c['number']}")
            chapters.append(dict(c, start=start, end=end,
                                 path=str(Path(EDITION) / plan['folder'] / filename(c['number'], c['title'])),
                                 raw=''.join(lines[start-1:end-1])))
        if ''.join(c['raw'] for c in chapters) != text:
            raise ValueError('Chapter spans lose, duplicate or reorder source text')
        boundaries = [c['start'] for c in chapters]
        for block in code_blocks(text):
            if bisect_right(boundaries, block.start) != bisect_right(boundaries, block.end):
                raise ValueError(f"Chapter cuts a code/diagram fence at line {block.start}")
        books.append(dict(plan=plan, text=text, lines=lines, headings=heads,
                          chapters=chapters, boundaries=boundaries,
                          map=str(Path(EDITION) / plan['folder'] / '00 - Reading Map.md')))
    return books


def rewrite_body(book, chapter, books):
    """Only navigation and opaque citation placement change; code stays byte-identical."""
    paths = {}
    for b in books:
        p = Path(b['plan']['source_book'])
        for spelling in (str(p), str(p.with_suffix('')), p.name, p.stem):
            paths[key(spelling)] = b

    def replace(match):
        if match[2] is None:
            return match[0]
        raw = match[2]
        target, separator, label = raw.partition('|')
        target = target.rstrip('\\')  # Escaped table pipes are not part of an anchor.
        path, hashmark, anchor = target.partition('#')
        dest = book if not path else paths.get(key(path.rstrip('\\')))
        if dest is None:
            return match[0]
        destination = dest['map']
        if hashmark and anchor:
            if anchor.startswith('^'):
                found = [n for n, line in enumerate(dest['lines'], 1)
                         if re.search(r'(?:^|\s)' + re.escape(anchor) + r'\s*$', line)]
            else:
                found = [h['line'] for h in dest['headings'] if h['kind'] == 'heading'
                         and heading_key(h['heading']) == heading_key(anchor)]
            if not found:
                raise ValueError(f'Cannot locate original target {raw}')
            # Obsidian resolves repeated original heading names to the first occurrence.
            owner = bisect_right(dest['boundaries'], found[0]) - 1
            destination = dest['chapters'][owner]['path']
        label = label if separator else (anchor or Path(path).stem)
        return '[[' + str(Path(destination).with_suffix('')) + hashmark + anchor + '|' + label + ']]'

    visible = {n for n, _ in prose_lines(chapter['raw'])}
    lines = chapter['raw'].splitlines(keepends=True)
    for n in visible:
        line = INLINE_OR_WIKI.sub(replace, lines[n-1])
        line = line.replace('**operator[](i)**', '**`operator[](i)`**')
        line = rewrite_markdown(line, book, chapter, books)
        # Retain opaque citation evidence without attaching it to a MathJax delimiter.
        line = re.sub(r'(`+).*?\1|(:chatgpt-content-reference\{[^}\n]*\})',
                      lambda m: m[0] if m[1] else '\n<!-- Original citation marker: ' + m[2] + ' -->\n', line)
        lines[n-1] = line
    result = ''.join(lines)
    original_blocks = [(b.language, b.body) for b in code_blocks(chapter['raw'])]
    if original_blocks != [(b.language, b.body) for b in code_blocks(result)]:
        raise ValueError('A source code/diagram body changed')
    return result


MD_LINK = re.compile(r'(!?)\[([^\]\n]*)\]\((<[^>\n]+>|[^\s)\n]+)\)')


def heading_slug(title):
    title = re.sub(r'[^\w\s-]', '', title.replace('`', '').lower())
    return re.sub(r'\s', '-', title)


def rewrite_markdown(line, book, chapter, books):
    def replace(match):
        image, label, raw = match.groups()
        url = raw.strip('<>')
        if re.match(r'[a-zA-Z][a-zA-Z0-9+.-]*:|//', url):
            return match[0]
        path, hashmark, anchor = unquote(url).partition('#')
        origin = Path(book['plan']['source_book']).parent
        normalized = os.path.normpath(str(origin / path)) if path else book['plan']['source_book']
        dest = next((b for b in books if b['plan']['source_book'] == normalized), None)
        if dest and hashmark:
            found = next((h for h in dest['headings'] if h['kind'] == 'heading'
                          and (heading_slug(h['heading']) == anchor
                               or heading_key(h['heading']) == heading_key(anchor))), None)
            if found is None:
                raise ValueError('Cannot resolve source Markdown heading: ' + url)
            owner = bisect_right(dest['boundaries'], found['line']) - 1
            return image + '[[' + str(Path(dest['chapters'][owner]['path']).with_suffix('')) + '#' + found['heading'] + '|' + label + ']]'
        if dest and not hashmark:
            return image + link(dest['map'], label)
        relative = os.path.relpath(normalized, Path(chapter['path']).parent)
        return image + '[' + label + '](' + quote(relative, safe='/._-') + hashmark + quote(anchor) + ')'
    # Inline-code link syntax is literal source code, as are fenced examples.
    parts = re.split(r'(`+.*?`+)', line)
    return ''.join(part if part.startswith('`') else MD_LINK.sub(replace, part) for part in parts)


def render(root, plans):
    books = prepare(root, plans)
    output = {}
    manifest = {'schema': 1, 'page_estimates_only': True,
                'page_model': plans[0]['page_model'],
                'transformations': ['Retarget wiki and Markdown links between masters to chapter files',
                                    'Rebase relative Markdown asset and code paths for the deeper chapter folder',
                                    'Format bare bold operator[](i) as inline code so it is not an empty Markdown link',
                                    'Preserve opaque citation markers in HTML comments on separate lines'],
                'duplicate_heading_policy': 'First original heading occurrence, as in the unsplit source',
                'books': []}
    overview = ['# Chapter Edition\n',
                'A chapter-by-chapter reading copy of the three continuous masters. '
                'All original source spans remain in order; the masters remain authoritative and intact.\n',
                'Read one substantial chapter at a time. Opening bridges give the context for its first topic; '
                'the full lessons, equations, figures, examples and code follow. '
                'The reading maps provide goals and recall prompts.\n',
                '**Length:** roughly 50 estimated pages per chapter. These are reading-size estimates, '
                'not measured PDF pages: 400 prose words, 48 wrapped code lines or 36 equation/table lines per page, '
                'with allowances for headings and figures. Actual Obsidian, print and screen layouts vary.\n',
                '**Vault links:** Concepts, Supplementary and image assets are shared with the surrounding Master vault. '
                'Copy the vault along with this folder if you need those external companions.\n',
                '**Updates:** edit a master, then run `python3 tools/build_chapter_edition.py`; '
                '`--check` detects stale or altered chapter copies. Human-reviewed boundary plans live in '
                '`tools/chapter_plans`. Do not edit generated chapter bodies directly.\n']
    for book in books:
        plan = book['plan']
        overview.append(link(book['map'], plan['title']) + '\n')
        reading = [f"# {plan['title']} — Reading Map\n", plan['book_introduction'] + '\n',
                   link(plan['source_book'], 'Continuous master') + ' · '
                   + link(EDITION + '/README.md', 'All three books') + '\n',
                   '| Chapter | Estimated pages | What you will learn |\n|---|---:|---|\n']
        record = {'source': plan['source_book'], 'sha256': digest(book['text']),
                  'bytes': len(book['text'].encode()), 'chapters': []}
        for i, chapter in enumerate(book['chapters']):
            chapter['estimated_pages'] = estimate_pages(chapter['raw'])
            body = rewrite_body(book, chapter, books)
            nav = [link(book['map'], 'Reading map')]
            if i:
                nav.append(link(book['chapters'][i-1]['path'], 'Previous chapter'))
            if i+1 < len(book['chapters']):
                nav.append(link(book['chapters'][i+1]['path'], 'Next chapter'))
            navigation = ' · '.join(nav)
            page = (f"# {chapter['number']:02d} — {chapter['title']}\n\n{navigation}\n\n"
                    f"{chapter['entry_bridge']}\n\n**By the end:** {chapter['learning_goal']}\n\n"
                    f"**Recall question:** {chapter['recall_question']}\n\n"
                    f"<!-- Derived from {plan['source_book']}; source lines {chapter['start']}–{chapter['end']-1}; "
                    f"source-span SHA256 {digest(chapter['raw'])}. -->\n\n"
                    + BODY_START + body + ('\n' if not body.endswith('\n') else '') + BODY_END
                    + '\n\n---\n\n' + navigation + '\n')
            output[chapter['path']] = page
            goal = chapter['learning_goal'].replace('|', '\\|').replace('\n', ' ')
            reading.append(f"| {link(chapter['path'], str(chapter['number']).zfill(2) + ' — ' + chapter['title'])} "
                           f"| {chapter['estimated_pages']} | {goal} |\n")
            record['chapters'].append({'path': chapter['path'], 'start_line': chapter['start'],
                                       'end_line_exclusive': chapter['end'],
                                       'source_span_sha256': digest(chapter['raw']),
                                       'rendered_body_sha256': digest(body),
                                       'file_sha256': digest(page),
                                       'estimated_pages': chapter['estimated_pages']})
        output[book['map']] = '\n'.join(reading)
        manifest['books'].append(record)
    output[EDITION + '/README.md'] = '\n'.join(overview)
    output[EDITION + '/MANIFEST.json'] = json.dumps(manifest, indent=2, ensure_ascii=False) + '\n'
    return output, manifest


def check_local_markdown(root, output):
    errors = []
    for relative, text in output.items():
        if not relative.endswith('.md'):
            continue
        for number, line in prose_lines(text):
            for part in re.split(r'(`+.*?`+)', line):
                if part.startswith('`'):
                    continue
                for match in MD_LINK.finditer(part):
                    url = match[3].strip('<>')
                    if re.match(r'[a-zA-Z][a-zA-Z0-9+.-]*:|//', url):
                        continue
                    target = unquote(url).split('#', 1)[0]
                    destination = os.path.normpath(str(Path(relative).parent / target))
                    if destination not in output and not (root / destination).is_file():
                        errors.append(f'{relative}:{number}: missing Markdown target {url}')
    return errors


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=ROOT)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    plans = [json.loads(p.read_text()) for p in sorted((args.root / 'tools/chapter_plans').glob('*.json'))]
    if len(plans) != 3:
        parser.error('Exactly three reviewed book plans are required')
    output, manifest = render(args.root, plans)
    link_errors = check_local_markdown(args.root, output)
    if link_errors:
        raise ValueError('\n'.join(link_errors))
    existing = {str(p.relative_to(args.root)) for p in (args.root / EDITION).rglob('*') if p.is_file()}
    extra = sorted(existing - output.keys())
    if extra:
        raise ValueError(f'Unexpected chapter-edition files; inspect rather than delete: {extra}')
    stale = []
    for relative, expected in output.items():
        path = args.root / relative
        if args.check:
            if not path.exists() or path.read_text(encoding='utf-8') != expected:
                stale.append(relative)
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(expected, encoding='utf-8')
    if stale:
        print(json.dumps({'stale_files': stale}, indent=2))
        return 1
    print(json.dumps({'mode': 'check' if args.check else 'build', 'books': len(plans),
                      'chapters': sum(len(b['chapters']) for b in manifest['books']),
                      'source_bytes': sum(b['bytes'] for b in manifest['books']),
                      'files': len(output), 'errors': 0}))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
