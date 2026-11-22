"""Book Two EPUB 3 full reader-finish packaging regressions.

These tests do not need the 568-row private AST cache or the user's Mac.
They reproduce actual failure types from the 2026-11-20 complete EPUBCheck:
- RSC-005: duplicate landmarks; invalid ARIA role doc-landmarks
- OPF-015: stale mathml properties on equation-free split documents
- NAV-011: back matter TOC entries outside reading-spine order
"""
from __future__ import annotations

import unittest
from lxml import etree as ET
from book02_finish_epub import Editor, NS, OPF, X, EPUB, NCX, M, Q

def page(inner: str) -> bytes:
    return (f'<html xmlns="{X}" xmlns:epub="{EPUB}">'
            f'<head><title>Book Two</title></head><body>{inner}</body></html>'
            ).encode('utf-8')


class BookTwoPublicationEPUB3Regressions(unittest.TestCase):
    def test_split_fragments_have_exact_mathml_manifest_features(self):
        editor=object.__new__(Editor)
        doc_math=page(
            '<p><math xmlns="http://www.w3.org/1998/Math/MathML">'
            '<mi>x</mi></math></p>')
        doc_plain=page('<p>No equations on this continuation page.</p>')
        names=[
          'text/ch008-p08.xhtml',
          'text/ch010-p07.xhtml',
          'text/ch010-p10.xhtml',
          'text/ch016-p07.xhtml',
          'text/ch012-p04.xhtml',
        ]
        editor.data={
            'EPUB/'+name:(doc_math if name.endswith('ch012-p04.xhtml')
                          else doc_plain) for name in names
        }
        original=dict(editor.data)
        manifest=ET.Element(Q(OPF,'manifest'))
        for i,name in enumerate(names):
            props='mathml' if i<4 else None
            attrs={'id':f'section-{i}','href':name,
                   'media-type':'application/xhtml+xml'}
            if props:attrs['properties']=props
            ET.SubElement(manifest,Q(OPF,'item'),**attrs)
        editor.manifest=manifest
        editor.report={}
        editor.sync_mathml_manifest()
        self.assertEqual(editor.report['mathml_opf_flags_removed'],4)
        self.assertEqual(editor.report['mathml_opf_flags_added'],1)
        for item in manifest:
            has_math=bool(
                ET.fromstring(editor.data['EPUB/'+item.get('href')])
                  .xpath('.//m:math',namespaces=NS))
            declared='mathml' in item.get('properties','').split()
            self.assertEqual(has_math,declared)
        self.assertEqual(editor.data,original)

    def test_existing_landmarks_reused_and_all_toc_entries_in_spine_order(self):
        editor=object.__new__(Editor)
        editor.reading_order=[2,3,4,1]+list(range(5,17))
        originals=[f'text/ch{i:03}.xhtml' for i in range(6,22)]
        extras=[
            'list_figures','glossary','symbols','source_companion',
            'further_reading','book_three_bridge','workbook','answer_key',
            'index'
        ]
        back=[f'text/book02-{slug}.xhtml' for slug in extras]
        # The real spine puts all frontmatter first, then reordered chapters,
        # archival references, and backmatter in the actual reading order.
        reading=[f'text/ch{n+5:03}.xhtml' for n in editor.reading_order]
        paths=['text/cover.xhtml','text/title_page.xhtml',
               *[f'text/ch{i:03}.xhtml' for i in range(1,6)],
               *reading,'text/ch022.xhtml','text/ch023.xhtml',*back]
        opf=ET.Element(Q(OPF,'package'),nsmap={None:OPF})
        manifest=ET.SubElement(opf,Q(OPF,'manifest'))
        spine=ET.SubElement(opf,Q(OPF,'spine'))
        for i,path in enumerate(paths):
            ident=f'resource_{i}'
            ET.SubElement(manifest,Q(OPF,'item'),id=ident,
                          href=path,**{'media-type':'application/xhtml+xml'})
            ET.SubElement(spine,Q(OPF,'itemref'),idref=ident)
        editor.opf=opf
        editor.manifest=manifest
        editor.spine=spine
        editor.items={i.get('id'):i for i in manifest}
        editor.names=['EPUB/nav.xhtml','EPUB/toc.ncx']
        editor.changed=set()
        editor.report={}
        # Chapter entries intentionally start in original source order;
        # the original Pandoc landmarks already exist and MUST be reused.
        toc_li=''.join(
            f'<li><a href="{p}">{p}</a></li>' for p in originals)
        html=page(
            f'<nav id="toc" epub:type="toc"><ol>{toc_li}</ol></nav>'
            '<nav id="landmarks" epub:type="landmarks">'
            '<h2>Existing Pandoc landmarks</h2><ol><li>'
            '<a href="text/cover.xhtml">Cover</a></li></ol></nav>')
        ncx_li=''.join(
            '<navPoint id="old-%s"><navLabel><text>%s</text>'
            '</navLabel><content src="%s"/></navPoint>'
            %(i,p,p) for i,p in enumerate(originals))
        ncx=(f'<ncx xmlns="{NCX}"><head><meta name="dtb:depth" '
             f'content="1"/></head><navMap>{ncx_li}</navMap></ncx>'
             ).encode('utf-8')
        editor.data={'EPUB/nav.xhtml':html,'EPUB/toc.ncx':ncx}
        editor.clean_namespace_and_nav()

        nav=ET.fromstring(editor.data['EPUB/nav.xhtml'])
        landmarks=[n for n in nav.iter(Q(X,'nav'))
                   if 'landmarks' in n.get(Q(EPUB,'type'),'').split()]
        self.assertEqual(len(landmarks),1)
        self.assertIsNone(landmarks[0].get('role'))
        self.assertEqual(landmarks[0].get('id'),'landmarks')
        ranks={path:i for i,path in enumerate(paths)}
        toc=nav.xpath('.//x:nav[@id="toc"]/x:ol/x:li/x:a',namespaces=NS)
        positions=[ranks[a.get('href').split('#')[0]] for a in toc]
        self.assertEqual(positions,sorted(positions))
        self.assertEqual(len(toc),16+len(extras))
        ncx_root=ET.fromstring(editor.data['EPUB/toc.ncx'])
        ncx_links=ncx_root.xpath(
            './ncx:navMap/ncx:navPoint/ncx:content',namespaces=NS)
        ncx_positions=[ranks[x.get('src').split('#')[0]]
                       for x in ncx_links]
        self.assertEqual(ncx_positions,sorted(ncx_positions))
        self.assertEqual(len(ncx_links),16+len(extras))
        self.assertNotIn(b'ns0:',editor.data['EPUB/nav.xhtml'])
        self.assertNotIn(b'ns0:',editor.data['EPUB/toc.ncx'])


if __name__=='__main__':
    unittest.main()
