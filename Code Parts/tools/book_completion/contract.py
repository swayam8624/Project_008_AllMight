"""Parse the supplied curriculum as data; never promote discovery to mastery."""
from collections import Counter
from hashlib import sha256
import re


def parse_contract(text):
    parts = []
    page = None
    for line in text.splitlines():
        if match := re.match(r'\[Physical PDF page (\d+)\]', line):
            page = int(match[1])
        if match := re.match(r'^Part (\d+)\.\s+(.+)$', line):
            parts.append({'part': int(match[1]), 'title': match[2].strip(), 'pdf_page': page})
    ids = [p['part'] for p in parts]
    if sorted(ids) != list(range(1, 3001)) or len(set(ids)) != 3000:
        raise ValueError('Canonical parts must contain exactly 1–3000, once each')
    matches = list(re.finditer(r'^M(\d{3}) - (.+)$', text, re.M))
    chunks = []
    custody = Counter()
    for i, match in enumerate(matches):
        end = matches[i+1].start() if i+1 < len(matches) else text.find('\nPart 1.', match.end())
        block = text[match.start():end]
        owns = re.search(r'Owns: original Parts (\d+)-(\d+)', block)
        if owns is None:
            raise ValueError('Missing chunk ownership: ' + match[1])
        first, last = map(int, owns.groups())
        if not 1 <= first <= last <= 3000:
            raise ValueError('Invalid chunk range')
        title = block.split('Owns:', 1)[0]
        title = ' '.join(title.splitlines())[7:].strip()
        prerequisites = re.search(r'Prerequisite gate:\s*(.+)', block)
        chunk = {'chunk': 'M' + match[1], 'title': title, 'first_part': first,
                 'last_part': last, 'phase': (first-1)//100+1,
                 'prerequisite_chunk_ids': re.findall(r'M\d{3}', prerequisites[1]) if prerequisites else []}
        chunks.append(chunk)
        custody.update(range(first, last+1))
    if [c['chunk'] for c in chunks] != [f'M{i:03}' for i in range(1, 181)]:
        raise ValueError('Expected canonical M001–M180 in order')
    if custody != Counter(range(1, 3001)):
        raise ValueError('Chunk ownership must cover each original part exactly once')
    owners = {i: c for c in chunks for i in range(c['first_part'], c['last_part']+1)}
    for part in parts:
        part.update(phase=owners[part['part']]['phase'], chunk=owners[part['part']]['chunk'])
    phases = []
    core_start = text.rfind('\nPhase 1 -', 0, text.find('\nPart 1.'))
    core = text[core_start:]
    for match in re.finditer(r'^Phase (\d+) - (.*?)\s+\(Parts\s+(\d+)-(\d+)\)', core, re.M | re.S):
        phases.append({'phase': int(match[1]), 'title': ' '.join(match[2].split()),
                       'first_part': int(match[3]), 'last_part': int(match[4])})
    if [p['phase'] for p in phases] != list(range(1, 31)):
        raise ValueError('Expected original 30-phase contract')
    return {'schema': 1, 'source_text_sha256': sha256(text.encode()).hexdigest(),
            'phases': phases, 'chunks': chunks, 'parts': parts}
