"""Source-loss and cross-chapter navigation regression tests."""
from pathlib import Path
import sys
import tempfile
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from build_chapter_edition import anchors, prepare, render, BODY_START, BODY_END
from check_vault_links import audit


class ChapterEditionTests(unittest.TestCase):
    def fixture(self, root, text):
        (root / 'Continuous Notes').mkdir()
        (root / 'Continuous Notes/Book.md').write_text(text)
        heads = anchors(text)
        start = {'heading': 'BOOK_START', 'occurrence': 1}
        end = {'heading': 'BOOK_END', 'occurrence': 1}
        cut = next(h for h in heads if h['heading'] == 'Second')
        chapters = []
        for i, (a, b) in enumerate([(start, cut), (cut, end)], 1):
            chapters.append(dict(number=i, start_anchor=a, end_anchor=b,
                                 title=f'Chapter {i}', entry_bridge='A specific bridge.',
                                 learning_goal='Understand the invariant.', recall_question='Why does it hold?',
                                 estimated_pages=50))
        return [dict(source_book='Continuous Notes/Book.md', folder='01 - Book',
                     title='Book', book_introduction='Read in order.', page_model='estimate', chapters=chapters)]

    def test_complete_source_order_and_cross_chapter_links(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            text = '# First\nRead [[#Second|the next topic]].\n## Second\nReturn [[Continuous Notes/Book#First]].\n'
            plans = self.fixture(root, text)
            output, manifest = render(root, plans)
            self.assertEqual(''.join(c['raw'] for c in prepare(root, plans)[0]['chapters']), text)
            for name, body in output.items():
                p = root / name
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_text(body)
            self.assertEqual(audit(root)['error_count'], 0)
            self.assertIn('Chapter Edition/01 - Book/02 - Chapter 2#Second|the next topic', output['Chapter Edition/01 - Book/01 - Chapter 1.md'])
            self.assertEqual(len(manifest['books'][0]['chapters']), 2)

    def test_fence_and_citation_content_preserved(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            text = '# First\n$$\nx=1\n$$ :chatgpt-content-reference{index="4"}\n```cpp\n// [[#Second]] :chatgpt-content-reference{index="2"}\n```\n## Second\nDone.\n'
            output, _ = render(root, self.fixture(root, text))
            chapter = output['Chapter Edition/01 - Book/01 - Chapter 1.md']
            self.assertIn('$$ \n<!-- Original citation marker:', chapter)
            self.assertIn('// [[#Second]] :chatgpt-content-reference{index="2"}', chapter)
            self.assertIn(BODY_START, chapter)
            self.assertIn(BODY_END, chapter)

    def test_escaped_table_alias_and_block_navigation(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            text = '# First\n| [[#Second\\|Next]] | [[#^answer]] |\n## Second\nAn answer. ^answer\n'
            output, _ = render(root, self.fixture(root, text))
            page = output['Chapter Edition/01 - Book/01 - Chapter 1.md']
            self.assertIn('02 - Chapter 2#Second|Next', page)
            self.assertIn('02 - Chapter 2#^answer|^answer', page)

    def test_relative_markdown_assets_and_heading_links(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            text = '# First\n**operator[](i)** [Continue](#second) ![Diagram](../Visual%20Recall/picture.svg) [Code](../tools/lab.cpp)\n## Second\nDone.\n'
            output, _ = render(root, self.fixture(root, text))
            page = output['Chapter Edition/01 - Book/01 - Chapter 1.md']
            self.assertIn('02 - Chapter 2#Second|Continue', page)
            self.assertIn('(../../Visual%20Recall/picture.svg)', page)
            self.assertIn('(../../tools/lab.cpp)', page)
            self.assertIn('**`operator[](i)`**', page)

    def test_missing_boundary_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            plans = self.fixture(root, '# First\nA.\n## Second\nB.\n')
            plans[0]['chapters'][1]['end_anchor'] = {'heading': 'Missing', 'occurrence': 1}
            with self.assertRaisesRegex(ValueError, 'Unresolved'):
                render(root, plans)

    def test_missing_or_duplicate_source_span_fails(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            plans = self.fixture(root, '# First\nA.\n## Second\nB.\n')
            plans[0]['chapters'][1]['start_anchor'] = {'heading': 'BOOK_START', 'occurrence': 1}
            with self.assertRaisesRegex(ValueError, 'lose, duplicate or reorder'):
                render(root, plans)

    def test_heading_inside_formula_or_code_is_not_a_boundary(self):
        text = '# First\n$$\n# formula\n$$ :chatgpt-content-reference{index="1"}\n```text\n## literal\n```\n## Second\n'
        self.assertEqual([h['heading'] for h in anchors(text)], ['First', 'Second'])


if __name__ == '__main__':
    unittest.main()
