"""Prevent incomplete inventories or discovery results becoming completion claims."""
from copy import deepcopy
import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from check_book_completion import audit
from book_completion.contract import parse_contract

ROOT = Path(__file__).resolve().parents[2]


class CompletionTests(unittest.TestCase):
    def setUp(self):
        self.curriculum = json.loads((ROOT/'tools/book_completion/canonical_curriculum.json').read_text())
        self.ledger = {'parts': [dict(part=p['part'], title=p['title'],
                                     review_status='not_audited', content_complete=False,
                                     destinations=[]) for p in self.curriculum['parts']]}

    def test_discovery_does_not_close_a_part(self):
        self.ledger['parts'][0]['candidate_headings'] = ['Some matching heading']
        result = audit(ROOT, self.curriculum, self.ledger)
        self.assertEqual(result['content_complete_parts'], 0)
        self.assertEqual(result['reviewed_parts'], 0)
        self.assertFalse(result['content_complete'])

    def test_duplicate_and_missing_part_rejected(self):
        self.ledger['parts'][-1] = deepcopy(self.ledger['parts'][0])
        self.assertTrue(audit(ROOT, self.curriculum, self.ledger)['errors'])

    def test_unaudited_completion_rejected(self):
        self.ledger['parts'][0]['content_complete'] = True
        self.assertTrue(audit(ROOT, self.curriculum, self.ledger)['errors'])

    def test_title_drift_rejected(self):
        self.ledger['parts'][100]['title'] = 'A renamed topic'
        self.assertTrue(audit(ROOT, self.curriculum, self.ledger)['errors'])

    def test_stale_evidence_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root/'book.md').write_text('The corrected explanation.\n')
            row = self.ledger['parts'][0]
            row.update(review_status='bounded_review', destinations=[{
                'file':'book.md', 'exact_quote':'The previous explanation.',
                'sha256':hashlib.sha256(b'The previous explanation.').hexdigest()}])
            self.assertTrue(audit(root, self.curriculum, self.ledger)['errors'])

    def test_missing_gate_evidence_cannot_close_reviewed_part(self):
        row = self.ledger['parts'][0]
        row.update(review_status='reviewed', content_complete=True)
        self.assertTrue(audit(ROOT, self.curriculum, self.ledger)['errors'])

    def test_contract_rejects_missing_atomic_number_before_matching_chunks(self):
        with self.assertRaisesRegex(ValueError, '1–3000'):
            parse_contract('Part 1. First topic\nPart 1. Duplicated topic\n')


if __name__ == '__main__':
    unittest.main()
