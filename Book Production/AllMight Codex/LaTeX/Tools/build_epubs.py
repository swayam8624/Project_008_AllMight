#!/usr/bin/env python3
"""Build reflowable EPUBs from a frozen conversion generation's semantic ASTs."""
from __future__ import annotations

import argparse
import copy
import hashlib
import html
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import urllib.parse
import zipfile
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, "/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15")
import build_books as epub_helpers  # noqa: E402

from book02_epub_supplements import STAGE as BOOK02_STAGE, Book02EpubSupplement, BOOK02_EPUB_CSS
from book02_single_volume import epub_frontmatter, epub_references
from book02_svg_id_repair import repair_epub_duplicate_svg_image_ids


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def plain(nodes):
    out = []
    def walk(x):
        if isinstance(x, dict):
            t=x.get("t"); c=x.get("c")
            if t == "Str": out.append(str(c))
            elif t == "Space": out.append(" ")
            elif t in ("SoftBreak", "LineBreak"): out.append(" ")
            else: walk(c)
        elif isinstance(x, list):
            for y in x: walk(y)
    walk(nodes)
    return "".join(out)


def pandoc_attr(a):
    if isinstance(a, list) and len(a) == 3:
        return a[0], a[1], dict(a[2])
    return "", [], {}


def set_attr(a, ident=None, keyvals=None):
    if not isinstance(a, list) or len(a) != 3: return
    if ident is not None: a[0] = ident
    if keyvals is not None: a[2] = [[k,v] for k,v in keyvals.items()]


def escape_unknown_html_tags(epub):
    """Escape literal C++ angle-bracket tokens emitted as unknown HTML-like tags."""
    valid={"a","abbr","address","area","article","aside","audio","b","base","bdi","bdo","blockquote","body","br","button","canvas","caption","cite","code","col","colgroup","data","dd","del","details","dfn","div","dl","dt","em","embed","figcaption","figure","footer","form","h1","h2","h3","h4","h5","h6","head","header","hr","html","i","iframe","img","input","ins","kbd","label","legend","li","link","main","map","mark","math","menu","meta","meter","nav","noscript","object","ol","optgroup","option","output","p","param","picture","pre","progress","q","rp","rt","ruby","s","samp","script","section","select","small","source","span","strong","style","sub","summary","sup","svg","table","tbody","td","template","textarea","tfoot","th","thead","time","title","tr","track","u","ul","var","video","wbr","annotation","annotation-xml","semantics","mi","mn","mo","mtext","mrow","msup","msub","msubsup","mfrac","msqrt","mroot","mover","munder","munderover","mtable","mtr","mtd","mspace","mstyle","merror","mfenced","maction","mmultiscripts","mprescripts","menclose","mpadded","mphantom","mlongdiv","mstack","ms","maligngroup","malignmark","none","path","g","defs","clipPath","mask","rect","circle","ellipse","line","polyline","polygon","text","tspan","linearGradient","radialGradient","stop","use","symbol","marker","pattern","filter","feGaussianBlur","feOffset","feBlend","feColorMatrix","feMerge","feMergeNode"}
    pat=re.compile(rb"<(/?)([A-Za-z][A-Za-z0-9:_-]*)(?=[\s/>])")
    changed=0;tmp=Path(epub).with_name(Path(epub).name+".tagfix")
    with zipfile.ZipFile(epub,"r") as source:
        with zipfile.ZipFile(tmp,"w") as output:
            for info in source.infolist():
                data=source.read(info.filename)
                if info.filename.endswith((".xhtml",".html")):
                    def sub(m):
                        nonlocal_dummy=None
                        global_changed[0] += int(m.group(2).decode("ascii","ignore") not in valid)
                        return m.group(0) if m.group(2).decode("ascii","ignore") in valid else b"&lt;"+m.group(1)+m.group(2)
                    global_changed=[0]
                    data=pat.sub(sub,data);changed+=global_changed[0]
                output.writestr(info,data)
    if changed: tmp.replace(epub)
    else: tmp.unlink()
    return changed


def walk_nodes(x):
    if isinstance(x, dict):
        yield x
        for v in x.values(): yield from walk_nodes(v)
    elif isinstance(x, list):
        for v in x: yield from walk_nodes(v)


def header_rows(ast):
    return [(n["c"][1][0], plain(n["c"][2])) for n in walk_nodes(ast) if n.get("t") == "Header" and n["c"][1][0]]


def load_ast(root, row):
    p = root / row["volume"] / "AST" / (Path(row["tex"]).stem + ".json")
    raw = p.read_bytes()
    actual = sha(raw)
    if actual != row["transformed_ast_sha256"]:
        raise RuntimeError(f"AST hash mismatch for {row['volume']}/{p.name}: {actual} != {row['transformed_ast_sha256']}")
    ast = json.loads(raw)
    if ast.get("pandoc-api-version", [0])[0] < 1:
        raise RuntimeError(f"Unexpected Pandoc AST at {p}")
    return ast


def ordered_rows(manifest, stage):
    rows=[r for r in manifest["rows"] if r["stage_id"] == stage["id"]]
    chapter={r["id"]:r for r in rows if r["kind"] == "chapter"}
    refs=defaultdict(list)
    for r in rows:
        if r["kind"] == "reference": refs[r.get("first_use_chapter")].append(r)
    ordered=[]; used=set()
    for cid in stage["chapters"]:
        if cid in chapter:
            ordered.append(chapter[cid]);used.add(chapter[cid]["id"])
        for r in sorted(refs.get(cid,[]),key=lambda x:(x.get("part_id", ""),x["id"])):
            if r["id"] not in used: ordered.append(r);used.add(r["id"])
    for r in sorted(rows,key=lambda x:(x.get("part_id", ""),x["id"])):
        if r["id"] not in used: ordered.append(r);used.add(r["id"])
    return ordered


def build_indexes(manifest, root):
    all_headers={}; owner={}; chapter_owner={}; chapter_title={}; target_anchor={}
    ast_cache={}
    for row in manifest["rows"]:
        ast=load_ast(root,row);ast_cache[row["id"]]=ast
        for ident,title in header_rows(ast):
            if ident in owner and owner[ident] != row["stage_id"]:
                raise RuntimeError(f"Duplicate heading ID across sources: {ident}")
            owner[ident]=row["stage_id"];all_headers[ident]=title
        if row["kind"] == "chapter":
            chapter_owner[row["id"]]=row["stage_id"]
            hs=header_rows(ast)
            chapter_title[row["id"]]=next((t for i,t in hs if i==row["id"]),row["id"])
            target_anchor[row["id"]]=hs[0][0] if hs else row["id"]
            owner[row["id"]]=row["stage_id"]
        elif row["kind"] == "reference":
            hs=header_rows(ast)
            if hs:
                target_anchor[row["id"]]=hs[0][0]
                owner[row["id"]]=row["stage_id"]
    return ast_cache,owner,all_headers,chapter_owner,chapter_title,target_anchor


class Packager:
    def __init__(self, root, out, manifest, ast_cache, owner, titles, chapter_owner, chapter_title, target_anchor):
        self.root=root;self.out=out;self.manifest=manifest;self.ast_cache=ast_cache
        self.owner=owner;self.titles=titles;self.chapter_owner=chapter_owner;self.chapter_title=chapter_title;self.target_anchor=target_anchor
        self.cross_stage_refs=defaultdict(dict)
        self.asset_dir=out/"assets";self.asset_dir.mkdir(parents=True,exist_ok=True)
        self.math_cache=ROOT/"Build"/"epub-mathml-cache";self.math_cache.mkdir(parents=True,exist_ok=True)
        self.asset_manifest={};self.attachment_by_source={};self.attachments={}
        # Supplementary EPUB adapter is lazy: the six unrelated stages do
        # not need Book Two visual libraries or regenerated assets.
        self.book02_supplement=None
        self.book02_cover_image=None
        for raw in manifest.get("source_attachment_inventory",{}).get("records",[]):
            rec=dict(raw);rec["sha256"]=rec.get("source_sha256");rec["name"]=(rec.get("source_names") or [Path(rec["source"]).name])[0]
            self.attachments[rec["id"]]=rec
            for p in rec.get("source_paths",[rec["source"]]):
                self.attachment_by_source[str(Path(p).resolve())]=rec

    def mathml(self, tex, display):
        key=json.dumps({"display":bool(display),"tex":tex,"v":1},sort_keys=True,ensure_ascii=False)
        ident=sha(key.encode())[:24]
        cached=Path("/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15/private-parser-build-3/sharedAssets/math")/(ident+".mml")
        dest=self.math_cache/(ident+".mml")
        if dest.is_file(): raw=dest.read_text(encoding="utf-8")
        elif cached.is_file():
            raw=cached.read_text(encoding="utf-8");dest.write_text(raw,encoding="utf-8")
        else:
            if r"\texttt{Occluded(ray,d-\epsilon)}." in tex:
                raw='<math xmlns="http://www.w3.org/1998/Math/MathML"><mrow><mtext>Occluded(ray,d−</mtext><mi>ε</mi><mtext>).</mtext></mrow><annotation encoding="application/x-tex">'+html.escape(tex)+'</annotation></math>'
            else:
                node={"t":"Math","c":[{"t":"DisplayMath" if display else "InlineMath"},tex]}
                block={"t":"Para","c":[node]}
                doc={"pandoc-api-version":[1,23,1],"meta":{},"blocks":[block]}
                p=subprocess.run([shutil.which("pandoc") or "pandoc","--from=json","--to=html5","--math-method=mathml"],input=json.dumps(doc,ensure_ascii=False),text=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
                if p.returncode or "class=\"math" in p.stdout or "Could not convert TeX math" in p.stderr:
                    raise RuntimeError(f"No native MathML conversion for {ident}: {tex!r}: {p.stderr[-700:]}")
                match=re.search(r"<math\b.*?</math>",p.stdout,re.S)
                if not match: raise RuntimeError(f"Pandoc produced no MathML for {ident}: {tex!r}")
                raw=match.group(0)
            dest.write_text(raw,encoding="utf-8")
        return epub_helpers.normalize_epub_mathml_presentation(raw)

    def mathml_ast(self, ast):
        replacements=0
        def math_node(n, as_block=False):
            nonlocal replacements
            display=n["c"][0]["t"]=="DisplayMath";raw=self.mathml(n["c"][1],display);replacements+=1
            return {"t":"RawBlock" if display and as_block else "RawInline","c":["html",raw]}
        def para(block):
            nonlocal replacements
            output=[];run=[];kind=block["t"]
            def flush():
                if run: output.append({"t":kind,"c":run[:]});run.clear()
            for n in block.get("c",[]):
                if isinstance(n,dict) and n.get("t")=="Math" and n["c"][0]["t"]=="DisplayMath":
                    flush();output.append(math_node(n,True))
                else: run.append(walk(n))
            flush();return output
        def walk(x):
            if isinstance(x,dict):
                if x.get("t")=="Math":return math_node(x)
                if x.get("t") in ("Para","Plain"):return para(x)
                return {k:walk(v) for k,v in x.items()}
            if isinstance(x,list):
                out=[]
                for item in x:
                    value=walk(item)
                    if isinstance(value,list) and isinstance(item,dict) and item.get("t") in ("Para","Plain"):out.extend(value)
                    else:out.append(value)
                return out
            return x
        result=walk(ast)
        return result,replacements

    def asset(self, source: Path, preferred_ext=None):
        source=source.resolve()
        if not source.is_file(): raise RuntimeError(f"Missing EPUB asset: {source}")
        raw=source.read_bytes(); digest=sha(raw)
        ext=(preferred_ext or source.suffix).lower()
        if ext not in (".svg",".png",".jpg",".jpeg",".gif",".webp"): raise RuntimeError(f"Unsafe EPUB image extension {ext}: {source}")
        name=f"{digest}{ext}"
        dest=self.asset_dir/name
        if not dest.exists(): dest.write_bytes(raw)
        self.asset_manifest[name]={"sha256":digest,"bytes":len(raw),"source":str(source)}
        return "assets/"+name

    def raster_pdf(self, source: Path):
        source=source.resolve()
        if not source.is_file(): raise RuntimeError(f"Missing diagram PDF: {source}")
        digest=sha(source.read_bytes()); dest=self.asset_dir/(digest+"-300dpi.png")
        if not dest.exists():
            exe=shutil.which("pdftoppm") or "/Users/swayamsingal/.cache/codex-runtimes/codex-primary-runtime/dependencies/bin/override/pdftoppm"
            if not Path(exe).exists(): raise RuntimeError(f"pdftoppm unavailable for {source}")
            prefix=self.asset_dir/(".render-"+digest)
            subprocess.run([exe,"-png","-r","300","-f","1","-l","1",str(source),str(prefix)],check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
            generated=Path(str(prefix)+"-1.png")
            if not generated.is_file(): raise RuntimeError(f"pdftoppm did not produce image for {source}")
            generated.replace(dest)
        raw=dest.read_bytes(); self.asset_manifest[dest.name]={"sha256":sha(raw),"bytes":len(raw),"source":str(source),"rasterized_pdf_sha256":digest}
        return "assets/"+dest.name

    def diagram_images(self, node, row):
        target=node["c"][2][0]; frag=urllib.parse.urlsplit(target).fragment
        name=Path(urllib.parse.unquote(urllib.parse.urlsplit(target).path)).name
        stem=Path(name).stem
        matches=[]
        for d in row.get("diagrams",[]):
            if d.get("id")==stem or str(d.get("bundled_pdf_sha256",""))==stem or Path(d.get("pdf","")).name==name:
                matches.append(d)
        if matches:
            d=matches[0]
            panels=d.get("panel_assets") or []
            if panels:
                rendered=[]
                for panel in panels:
                    svg_path=panel.get("print_svg")
                    if svg_path:
                        if "/current/Assets/Print/" in svg_path:
                            sp=self.root/"Assets"/"Print"/Path(svg_path).name
                        else:
                            sp=(self.root/svg_path).resolve() if not Path(svg_path).is_absolute() else Path(svg_path)
                        rendered.append(self.asset(sp,".svg"))
                    else:
                        p=panel.get("print_pdf")
                        if "/current/Assets/Print/" in p:
                            pp=self.root/"Assets"/"Print"/Path(p).name
                        else:
                            pp=(self.root/p).resolve() if not Path(p).is_absolute() else Path(p)
                        rendered.append(self.raster_pdf(pp))
                if len(rendered)==1: return rendered
                return rendered
            for key in ("epub_svg","svg","source_svg"):
                if d.get(key):
                    p=Path(d[key]);p=p if p.is_absolute() else self.root/p
                    if p.suffix.lower()==".svg" and p.is_file(): return [self.asset(p,".svg")]
            pdf=Path(d.get("pdf", "")); pdf=pdf if pdf.is_absolute() else self.root/pdf
            return [self.raster_pdf(pdf)]
        p=(self.root/urllib.parse.unquote(urllib.parse.urlsplit(target).path)).resolve()
        if "/current/Assets/Print/" in urllib.parse.unquote(urllib.parse.urlsplit(target).path):
            p=self.root/"Assets"/"Print"/Path(name).name
        if p.suffix.lower()==".pdf": return [self.raster_pdf(p)]
        if p.is_file() and p.suffix.lower() in (".svg",".png",".jpg",".jpeg",".gif",".webp"):
            return [self.asset(p)]
        # The normalized image target names the original print-PDF hash, bundled in the current generation.
        if re.fullmatch(r"[0-9a-f]{64}",stem):
            candidate=self.root/"Assets"/"Print"/name
            if candidate.is_file() and candidate.suffix.lower()==".png": return [self.asset(candidate)]
            pdf=self.root/"Assets"/"Print"/(stem+".pdf")
            if pdf.is_file(): return [self.raster_pdf(pdf)]
        raise RuntimeError(f"No EPUB-safe asset for image target {target!r} in {row['volume']}")

    def attachment_for(self, row, target):
        u=urllib.parse.urlsplit(target); path=urllib.parse.unquote(u.path)
        if not path: return None
        source=Path(path)
        if not source.is_absolute():
            source=(Path(row["source"]).resolve().parent/source).resolve()
        return self.attachment_by_source.get(str(source.resolve()))

    def transform(self, ast, row, mode, stage_id):
        doc=copy.deepcopy(ast); links_cross={}; unresolved=[]
        def walk(x):
            if isinstance(x,dict):
                typ=x.get("t")
                if typ=="Image":
                    srcs=self.diagram_images(x,row)
                    if len(srcs)==1: x["c"][2][0]=srcs[0]
                    else:
                        return {"t":"Span","c":[["",["diagram-panels"],[]],[{"t":"Image","c":[x["c"][0],x["c"][1],[srcs[0],""]]}]+[{"t":"Image","c":[["",[],[]],[],[s,""]]} for s in srcs[1:]]]}
                if typ=="Link":
                    target=x["c"][2][0];u=urllib.parse.urlsplit(target)
                    if u.scheme or target.startswith("//"): pass
                    else:
                        fragment=urllib.parse.unquote(u.fragment)
                        # Hyperref's PDF URL form stores a real target as nameddest=ID.
                        if fragment.startswith("nameddest="): fragment=fragment.split("=",1)[1]
                        if fragment:
                            dest=fragment
                            if dest == "volume-reading-map":
                                x["c"][2][0]="#"+(stage_id if mode=="collection" else "volume-reading-map")
                                return x
                            real_dest=self.target_anchor.get(dest,dest)
                            if mode=="collection":
                                if dest in self.owner: x["c"][2][0]="#"+real_dest
                                elif dest in self.attachments: x["c"][2][0]="#source-"+dest
                                else: unresolved.append({"source":row["id"],"target":target,"reason":"unmapped fragment"})
                            else:
                                target_stage=self.owner.get(dest)
                                if target_stage is None and dest.startswith("appendix-"):
                                    target_stage=next((s["id"] for s in self.manifest["stages"] if any(r["id"]==dest and r["stage_id"]==s["id"] for r in self.manifest["rows"])),None)
                                if target_stage==stage_id: x["c"][2][0]="#"+real_dest
                                elif target_stage:
                                    key=(target_stage,real_dest);ref="cross-stage-"+sha((target_stage+"\0"+real_dest).encode())[:18]
                                    links_cross[key]=ref;x["c"][2][0]="#"+ref
                                elif dest in self.attachments:
                                    rec=self.attachments[dest]
                                    if rec.get("stage_id")==stage_id: x["c"][2][0]="#source-"+dest
                                    else:
                                        owner_stage=rec["stage_id"];ref="cross-stage-source-"+dest
                                        links_cross[(owner_stage,"source-"+dest)]=ref;x["c"][2][0]="#"+ref
                                else: unresolved.append({"source":row["id"],"target":target,"reason":"unmapped fragment"})
                        elif u.path:
                            suffix=Path(u.path).suffix.lower()
                            if suffix in (".svg",".png",".jpg",".jpeg",".gif",".webp"):
                                if "/current/Assets/Print/" in urllib.parse.unquote(u.path):
                                    media=self.root/"Assets"/"Print"/Path(urllib.parse.unquote(u.path)).name
                                else:
                                    media=(Path(row["source"]).resolve().parent/urllib.parse.unquote(u.path)).resolve()
                                if media.is_file():
                                    x["t"]="Image";x["c"][2]=[self.asset(media),u.fragment]
                                else: unresolved.append({"source":row["id"],"target":target,"reason":"linked media file missing"})
                                return x
                            rec=self.attachment_for(row,target)
                            if rec:
                                if mode=="collection" or rec.get("stage_id")==stage_id:
                                    x["c"][2][0]="#source-"+rec["id"]
                                else:
                                    owner_stage=rec["stage_id"];ref="cross-stage-source-"+rec["id"]
                                    links_cross[(owner_stage,"source-"+rec["id"])]=ref;x["c"][2][0]="#"+ref
                            elif Path(u.path).suffix.lower() not in (".pdf",):
                                unresolved.append({"source":row["id"],"target":target,"reason":"unrouted local link"})
                for k,v in list(x.items()):
                    y=walk(v)
                    if y is not None: x[k]=y
                return x
            if isinstance(x,list):
                out=[]
                for y in x:
                    z=walk(y)
                    out.append(z)
                return out
            return x
        doc["blocks"]=walk(doc.get("blocks",[]))
        if unresolved: raise RuntimeError(f"Unrouted EPUB links in {row['id']}: {unresolved[:5]}")
        return doc,links_cross

    def attachment_blocks(self, stage_ids):
        blocks=[]; included=[]
        for rec in sorted(self.attachments.values(),key=lambda r:(r.get("name","").casefold(),r["id"])):
            if rec.get("stage_id") not in stage_ids: continue
            src=Path(rec["source"]);raw=src.read_bytes()
            if sha(raw)!=rec["sha256"]: raise RuntimeError(f"Source attachment hash mismatch: {src}")
            code=raw.decode("utf-8")
            lang=src.suffix.lower().lstrip(".") or "text"
            blocks.extend([{"t":"Header","c":[2,["source-"+rec["id"],[],[]],[{"t":"Str","c":rec.get("name",src.name)}]]},{"t":"CodeBlock","c":[["",[lang],[]],code]}])
            included.append({"id":rec["id"],"sha256":rec["sha256"],"name":rec.get("name",src.name),"stage_id":rec["stage_id"]})
        return blocks,included

    def compose(self, mode, stage=None):
        selected=[stage] if stage else self.manifest["stages"]
        blocks=[]; included=[]; cross={}
        if mode=="stage":
            blocks.append({"t":"Header","c":[1,["volume-reading-map",[],[]],[{"t":"Str","c":"Reading Map"}]]})
            blocks.append({"t":"Para","c":[{"t":"Str","c":"Chapters and companion references appear in approved reading order. Source links resolve to the preserved attachments at the end of this volume."}]})
        for st in selected:
            if mode=="collection":
                blocks.append({"t":"Header","c":[1,[st["id"],[],[]],[{"t":"Str","c":st["title"]}] ]})
            if st["id"]==BOOK02_STAGE:
                if mode=="stage":
                    blocks[:0]=epub_frontmatter()
                else:
                    blocks.extend(epub_frontmatter())
            for row in ordered_rows(self.manifest,st):
                if st["id"]==BOOK02_STAGE and row["kind"]=="chapter":
                    if self.book02_supplement is None:
                        self.book02_supplement=Book02EpubSupplement(self.out,self.asset)
                    if row["id"]=="01-foundations-i-ch017":
                        blocks.extend(self.book02_supplement.chapter_blocks(
                            row["id"],mode,prelude=True))
                ast=self.ast_cache[row["id"]]
                transformed,refs=self.transform(ast,row,mode,st["id"]);cross.update(refs)
                rb=transformed.get("blocks",[])
                for node in walk_nodes(rb):
                    if node.get("t")=="Header":
                        if mode=="collection": node["c"][0]=min(6,int(node["c"][0])+1)
                        elif row["kind"]=="reference" and node["c"][0]==1: node["c"][0]=2
                blocks.extend(rb);included.append({"id":row["id"],"kind":row["kind"],"stage_id":st["id"],"ast_sha256":row["transformed_ast_sha256"]})
                # Semantically identical location to the print build insertion
                # (after this original chapter, never replacing its AST).
                if st["id"]==BOOK02_STAGE and row["kind"]=="chapter":
                    blocks.extend(self.book02_supplement.chapter_blocks(row["id"],mode))
            if st["id"]==BOOK02_STAGE:
                blocks.extend(epub_references())
        if mode=="stage" and cross:
            blocks.extend([{"t":"Header","c":[1,["cross-stage-reference-index",[],[]],[{"t":"Str","c":"Cross Stage References"}]]},{"t":"Para","c":[{"t":"Str","c":"Each entry links to the corresponding topic in its stage EPUB. The chapter and heading provide a reading locator."}]}])
            for (peer,dest),ref in sorted(cross.items()):
                st=next(s for s in self.manifest["stages"] if s["id"]==peer)
                ch=next((cid for cid in st["chapters"] if dest==cid or dest.startswith(cid+"-")),None)
                if ch: locator=f"{self.chapter_title.get(ch,ch)} — {self.titles.get(dest,dest)}"
                else: locator=self.titles.get(dest,dest)
                href=f"{peer}.epub#{urllib.parse.quote(dest)}"
                self.cross_stage_refs[stage["id"]][ref]={"peer":peer,"target":dest,"locator":locator,"peer_title":st["title"]}
                blocks.extend([{"t":"Header","c":[2,[ref,[],[]],[{"t":"Str","c":locator}]]},{"t":"Para","c":[{"t":"Str","c":f"Peer EPUB: {peer}.epub. Topic target: {dest}. CFI pointer will be listed here after all stage books are built."}]}])
        attach,attached=self.attachment_blocks({s["id"] for s in selected});blocks.extend(attach)
        return {"pandoc-api-version":[1,23,1],"meta":{},"blocks":blocks},included,attached

    def run_build(self, generation, stage=None):
        mode="stage" if stage else "collection"
        label=stage["id"] if stage else "AllMight-Codex-Collection"
        ast,rows,attachments=self.compose(mode,stage)
        ast,presentation_report=epub_helpers.presentation_html_clone(ast,label)
        # Existing 568 frozen sources remain SHA-locked. Added Book02 math
        # must also be counted when validating packaged native MathML.
        expected_math_in_final_ast=sum(1 for n in walk_nodes(ast) if n.get("t")=="Math")
        ast,math_replacements=self.mathml_ast(ast)
        content=self.out/(label+".json");content.write_text(json.dumps(ast,ensure_ascii=False),encoding="utf-8")
        css=self.out/"epub.css"
        css_text=epub_helpers.FINAL_CSS+"\nspan.diagram-panels { display: block; }\nspan.diagram-panels img { display: block; margin: .8em auto; }\npre > code { display: block; min-width: 0; max-width: 100%; white-space: pre-wrap !important; overflow-wrap: anywhere; }\npre > code.sourceCode { white-space: pre-wrap !important; }\npre > code.sourceCode > span { display: block !important; line-height: 1.25; white-space: inherit; overflow-wrap: anywhere; }\n"
        # The Book Two layout is additive and stage-scoped: do not restyle
        # the six other volumes or modify the Book One reference EPUB.
        if stage is None or stage["id"] == BOOK02_STAGE:
            css_text += BOOK02_EPUB_CSS
        css.write_text(css_text,encoding="utf-8")
        output=self.out/(label+".epub")
        title=("From Equations to Geometry" if stage and stage["id"]==BOOK02_STAGE
               else stage["title"] if stage else "AllMight Codex — Complete Collection")
        split_level="2" if mode=="collection" else "1"
        cmd=[shutil.which("pandoc") or "pandoc",str(content),"--from=json","--to=epub3","--output",str(output),"--css",str(css),f"--split-level={split_level}","--metadata",f"title={title}","--metadata","author=Swayam Singal","--metadata","lang=en"]
        if stage and stage["id"]==BOOK02_STAGE:
            cmd.extend(["--metadata", "subtitle=Mathematics, Geometry, and Computational Foundations for Graphics, Simulation, and Research"])
            if self.book02_cover_image is not None:
                cmd.extend(["--epub-cover-image", str(self.book02_cover_image)])
        p=subprocess.run(cmd,cwd=self.out,text=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        if p.returncode: raise RuntimeError(f"Pandoc EPUB build failed for {label}: {p.stderr[-4000:]}")
        unknown_tags_escaped=escape_unknown_html_tags(output)
        # Pandoc may pack more than one scientific raster <image> element
        # with the same generated SVG id (EPUBCheck RSC-005, file40.svg).
        # Rename only unreferenced, duplicate image ids; preserve XML pixels
        # and every other EPUB member. Abort rather than guess on references.
        if stage is None or stage["id"] == BOOK02_STAGE:
            svg_id_repairs=repair_epub_duplicate_svg_image_ids(output)
            if svg_id_repairs["id_renames"]:
                print("BOOK02_SVG_DUPLICATE_IDS_FIXED",
                      svg_id_repairs["id_renames"],
                      svg_id_repairs["details"],flush=True)
        else:
            svg_id_repairs={"svg_files_repaired":0,"id_renames":0,"details":{}}
        epub_helpers.declare_epub_mathml_properties(output)
        expected=expected_math_in_final_ast
        if math_replacements != expected:
            raise RuntimeError(f"Incomplete Book02/EPUB MathML conversion: {math_replacements} of {expected}")
        package=epub_helpers.validate_epub_package(output,expected)
        epubcheck="/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15/epubcheck/epubcheck-5.4.0/epubcheck.jar"
        java=shutil.which("java")
        if not java: raise RuntimeError("Java runtime is unavailable; cannot run pinned EPUBCheck 5.4.0")
        ec=subprocess.run([java,"-jar",epubcheck,str(output)],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        report={"book02_cover_sha256":(sha(self.book02_cover_image.read_bytes()) if stage and stage["id"]==BOOK02_STAGE and self.book02_cover_image is not None else None),"generation_id":generation,"artifact":output.name,"path":str(output.resolve()),"sha256":sha(output.read_bytes()),"bytes":output.stat().st_size,"stage_id":stage["id"] if stage else None,"source_rows":len(rows),"chapter_rows":sum(x["kind"]=="chapter" for x in rows),"reference_rows":sum(x["kind"]=="reference" for x in rows),"source_attachments":len(attachments),"source_attachment_sha256":attachments,"expected_math_nodes":expected,"converted_math_nodes":math_replacements,"presentation_clone":presentation_report,"book02_supplement_sources":(self.book02_supplement.used if self.book02_supplement is not None else []),"unknown_html_like_tokens_escaped":unknown_tags_escaped,"svg_id_repairs":svg_id_repairs,"package_check":package,"epubcheck":{"version":"5.4.0","exit_code":ec.returncode,"output":ec.stdout[-2500:]},"pandoc_stderr_tail":p.stderr[-1000:]}
        return report


CFI_INDEX_CACHE={}


def epub_cfi_index(epub_path):
    """Index element IDs to CFIs from actual OPF spine and XHTML element paths."""
    import posixpath
    key=str(Path(epub_path).resolve())
    if key in CFI_INDEX_CACHE:return CFI_INDEX_CACHE[key]
    by_id={};by_cfi={}
    with zipfile.ZipFile(epub_path) as zf:
        container=epub_helpers.ET.fromstring(zf.read("META-INF/container.xml"))
        opf_path=next(n.attrib.get("full-path") for n in container.iter() if n.tag.endswith("rootfile"))
        opf=epub_helpers.ET.fromstring(zf.read(opf_path));base=posixpath.dirname(opf_path)
        items={n.attrib.get("id"):n.attrib for n in opf.iter() if n.tag.endswith("item")}
        spine=next(n for n in opf.iter() if n.tag.endswith("spine"));refs=[n for n in spine if n.tag.endswith("itemref")]
        for idx,itemref in enumerate(refs):
            item=items.get(itemref.attrib.get("idref"),{});href=urllib.parse.unquote(item.get("href",""));doc_path=posixpath.normpath(posixpath.join(base,href))
            if doc_path not in zf.namelist() or not doc_path.endswith((".xhtml",".html")):continue
            doc=epub_helpers.ET.fromstring(zf.read(doc_path));parent={child:node for node in doc.iter() for child in list(node)}
            body=next((n for n in doc.iter() if n.tag.endswith("body")),None)
            if body is None:continue
            for target in doc.iter():
                ident=target.attrib.get("id")
                if not ident:continue
                chain=[];node=target
                while node is not None and node is not body:
                    chain.append(node);node=parent.get(node)
                if node is not body:continue
                chain.reverse();steps=[4]
                for child in chain:
                    siblings=[n for n in list(parent[child]) if isinstance(n.tag,str)]
                    steps.append(2*(siblings.index(child)+1))
                path="/"+"/".join(map(str,steps[:-1]))+"/"+str(steps[-1])+"["+ident+"]"
                cfi=f"epubcfi(/6/{2*(idx+1)}[{itemref.attrib.get('idref')}]!{path})"
                by_id.setdefault(ident,cfi);by_cfi[cfi]=ident
    CFI_INDEX_CACHE[key]=(by_id,by_cfi)
    return by_id,by_cfi


def epub_target_cfi(epub_path, target_id):
    by_id,_=epub_cfi_index(epub_path)
    actual=target_id if target_id in by_id else "id_"+target_id
    if actual not in by_id:raise RuntimeError(f"Target ID {target_id!r} not found in {epub_path}")
    return by_id[actual]


def epub_actual_target_id(epub_path,target_id):
    by_id,_=epub_cfi_index(epub_path)
    actual=target_id if target_id in by_id else "id_"+target_id
    if actual not in by_id:raise RuntimeError(f"Target ID {target_id!r} not found in {epub_path}")
    return actual


def resolve_epub_cfi(epub_path, cfi):
    """Check a generated CFI maps back to an ID in the actual target publication."""
    _,by_cfi=epub_cfi_index(epub_path)
    if cfi not in by_cfi:raise RuntimeError(f"Generated CFI does not resolve to an actual element: {cfi}")
    return by_cfi[cfi]


def rewrite_cross_stage_cfis(epub_paths, stages, references):
    """Insert verified target EPUB CFI locators as readable text in stage reference indexes."""
    epub_helpers.ET.register_namespace("", "http://www.w3.org/1999/xhtml")
    epub_helpers.ET.register_namespace("epub", "http://www.idpf.org/2007/ops")
    by_id={s["id"]:Path(epub_paths[s["id"]]) for s in stages}
    reports={}
    for st in stages:
        src=by_id[st["id"]];tmp=src.with_name(src.name+".cfi-tmp");rewrites=[];refmap=references.get(st["id"],{})
        if not refmap:
            reports[st["id"]]=[]
            continue
        with zipfile.ZipFile(src,"r") as zin,zipfile.ZipFile(tmp,"w") as zout:
            for info in zin.infolist():
                payload=zin.read(info.filename)
                if info.filename.endswith((".xhtml",".html")):
                    doc=epub_helpers.ET.fromstring(payload);parent={child:node for node in doc.iter() for child in list(node)}
                    for ref,meta in refmap.items():
                        head=next((n for n in doc.iter() if n.attrib.get("id")==ref),None)
                        if head is None:continue
                        pnode=next((n for n in list(parent.get(head,[])) if n.tag.endswith("p")),None)
                        if pnode is None:continue
                        cfi=epub_target_cfi(by_id[meta["peer"]],meta["target"])
                        actual_id=epub_actual_target_id(by_id[meta["peer"]],meta["target"])
                        pnode.clear();pnode.attrib.update({"data-peer-epub":meta["peer"]+".epub","data-target-id":meta["target"],"data-epub-target-id":actual_id,"data-epub-cfi":cfi})
                        pnode.text=f"{meta['peer_title']}: {meta['locator']}. For a direct offline jump, use the complete collection EPUB."
                        rewrites.append({"peer":meta["peer"],"target":meta["target"],"cfi":cfi,"reference_id":ref})
                    payload=epub_helpers.ET.tostring(doc,encoding="utf-8",xml_declaration=True)
                zout.writestr(info,payload)
        tmp.replace(src);reports[st["id"]]=rewrites
    return reports


def patch_epub_project_css(epub, old_css, new_css):
    """Replace only the project's stylesheet entry while preserving all XHTML and OPF bytes."""
    import posixpath
    epub=Path(epub);tmp=epub.with_name(epub.name+".css-tmp");replaced=[]
    with zipfile.ZipFile(epub,"r") as zin:
        container=epub_helpers.ET.fromstring(zin.read("META-INF/container.xml"))
        opf_path=next(n.attrib.get("full-path") for n in container.iter() if n.tag.endswith("rootfile"))
        opf=epub_helpers.ET.fromstring(zin.read(opf_path));base=posixpath.dirname(opf_path)
        targets=set()
        for item in opf.iter():
            if not item.tag.endswith("item") or item.attrib.get("media-type")!="text/css":continue
            href=urllib.parse.unquote(item.attrib.get("href",""));target=posixpath.normpath(posixpath.join(base,href))
            payload=zin.read(target)
            if payload.lstrip().startswith(b"/* Reflow first:"):
                if sha(payload)!=sha(old_css):raise RuntimeError(f"Unexpected project CSS fingerprint in {epub}: {target}")
                targets.add(target)
        if len(targets)!=1:raise RuntimeError(f"Expected one project stylesheet in {epub}; found {len(targets)}")
        with zipfile.ZipFile(tmp,"w") as zout:
            for info in zin.infolist():
                payload=new_css if info.filename in targets else zin.read(info.filename)
                zout.writestr(info,payload)
    tmp.replace(epub)
    return {"path":str(epub.resolve()),"patched_css_paths":sorted(targets),"old_css_sha256":sha(old_css),"new_css_sha256":sha(new_css),"epub_sha256":sha(epub.read_bytes()),"epub_bytes":epub.stat().st_size}



def publish_epubs(out, destination, manifest, generation):
    """Publish only checked, fingerprint-matched books and reader-facing metadata."""
    report=json.loads((out/"epub-build-report.json").read_text(encoding="utf-8"))
    if report.get("generation_id") != generation:
        raise RuntimeError("Build report generation does not match requested generation")
    stages=manifest["stages"]
    records={r["stage_id"]:r for r in report["stage_epubs"]}
    if set(records) != {s["id"] for s in stages}:
        raise RuntimeError("Build report does not contain exactly the expected stages")
    items=[(s["id"]+".epub",records[s["id"]]) for s in stages]
    items.append(("AllMight-Codex-Collection.epub",report["collection_epub"]))
    artifacts=[]
    for name,record in items:
        source=out/name
        digest=sha(source.read_bytes())
        if digest != record.get("sha256") or record.get("epubcheck",{}).get("exit_code") != 0:
            raise RuntimeError(f"Unverified or changed EPUB: {source}")
        artifacts.append({"file":name,"sha256":digest,"bytes":source.stat().st_size})
    destination.mkdir(parents=True,exist_ok=True)
    allowed={name for name,_ in items}|{"INDEX.md","QA-summary.json"}
    extras=[p.name for p in destination.iterdir() if p.name not in allowed]
    if extras: raise RuntimeError(f"Unexpected reader-folder entries; preserve them elsewhere first: {extras}")
    for name,_ in items:
        temp=destination/("."+name+".tmp")
        shutil.copyfile(out/name,temp);temp.replace(destination/name)
    lines=["# AllMight Codex ebooks","","The complete collection contains all seven stages and supports offline links across them. Each stage is also available as a separate reflowable EPUB.","","## Reading order",""]
    for index,stage in enumerate(stages,1):
        lines.append(f"{index}. [{stage['title']}](<{stage['id']}.epub>)")
    lines += ["","[Complete collection](AllMight-Codex-Collection.epub)","","Standalone books identify cross-stage references by stage and topic. Use the collection for direct offline navigation across stages.",""]
    (destination/"INDEX.md").write_text("\n".join(lines),encoding="utf-8")
    summary={"generation_id":generation,"source_totals":report["source_totals"],"artifacts":artifacts,"epubcheck":"5.4.0: all eight passed","detailed_report":str((out/"epub-build-report.json").resolve())}
    (destination/"QA-summary.json").write_text(json.dumps(summary,indent=2,ensure_ascii=False)+"\n",encoding="utf-8")
    print(f"PUBLISHED generation={generation} artifacts={len(items)} destination={destination}",flush=True)


def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--generation",required=True,help="Generation ID, e.g. .generation-uuihb_gl")
    ap.add_argument("--output-dir",help="Output directory; defaults to Build/<generation>/epubs")
    ap.add_argument("--stage",help="Build only this exact stage ID")
    ap.add_argument("--book02-cover-image",help="Optional Book Two front-cover JPEG path for the EPUB only; no changes to print interior")
    ap.add_argument("--collection-only",action="store_true",help="Build only the all-stage collection EPUB")
    ap.add_argument("--finalize-crossrefs-only",action="store_true",help="Add verified CFI locators to already-built stage EPUBs without rerunning Pandoc")
    ap.add_argument("--patch-css-only",action="store_true",help="Patch the embedded project stylesheet in the eight existing EPUBs and revalidate without rerunning Pandoc")
    ap.add_argument("--publish",action="store_true",help="Publish checked books from the build directory to Books/Ebooks")
    args=ap.parse_args()
    gen=args.generation
    if "/" in gen or "\\" in gen or gen in (".",".."):
        ap.error("--generation must be a generation directory name")
    root=ROOT/"Generations"/gen
    if not root.is_dir(): ap.error(f"generation not found: {root}")
    manifest=json.loads((root/"conversion-manifest.json").read_text(encoding="utf-8"))
    if manifest.get("generation_id") != gen: raise RuntimeError(f"Generation manifest mismatch: expected {gen}, got {manifest.get('generation_id')}")
    if len(manifest.get("rows",[])) != 568: raise RuntimeError(f"Expected 568 normalized source rows, got {len(manifest.get('rows',[]))}")
    out=Path(args.output_dir).resolve() if args.output_dir else ROOT/"Build"/gen/"epubs"
    out.mkdir(parents=True,exist_ok=True)
    if args.publish:
        publish_epubs(out,ROOT/"Books"/"Ebooks",manifest,gen)
        return 0
    ast_cache,owner,titles,chapter_owner,chapter_title,target_anchor=build_indexes(manifest,root)
    all_math=sum(1 for ast in ast_cache.values() for n in walk_nodes(ast) if n.get("t")=="Math")
    if all_math != 39483: raise RuntimeError(f"Expected 39,483 transformed Math nodes, found {all_math}")
    builder=Packager(root,out,manifest,ast_cache,owner,titles,chapter_owner,chapter_title,target_anchor)
    if args.book02_cover_image:
        cover=Path(args.book02_cover_image).expanduser().resolve()
        if not cover.is_file() or cover.suffix.lower() not in (".jpg",".jpeg"):
            ap.error("--book02-cover-image must reference an existing JPEG; image not found or invalid extension")
        try:
            from PIL import Image
            with Image.open(cover) as img:
                img.verify()
            with Image.open(cover) as img:
                if img.format!="JPEG" or img.width<300 or img.height<450 or img.height<=img.width:
                    ap.error("Book Two EPUB cover must be a valid portrait JPEG at least 300 x 450 pixels")
        except (OSError,ValueError) as exc:
            ap.error("Cannot decode Book Two cover JPEG: "+str(exc))
        builder.book02_cover_image=cover
    stages=manifest["stages"]
    if args.patch_css_only:
        css=out/"epub.css"
        old_css=css.read_bytes()
        new_css=(epub_helpers.FINAL_CSS+"\nspan.diagram-panels { display: block; }\nspan.diagram-panels img { display: block; margin: .8em auto; }\npre > code { display: block; min-width: 0; max-width: 100%; white-space: pre-wrap !important; overflow-wrap: anywhere; }\npre > code.sourceCode { white-space: pre-wrap !important; }\npre > code.sourceCode > span { display: block !important; line-height: 1.25; white-space: inherit; overflow-wrap: anywhere; }\n").encode("utf-8")
        css.write_bytes(new_css)
        records=[]
        epubcheck="/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15/epubcheck/epubcheck-5.4.0/epubcheck.jar"
        for st in stages:
            path=out/(st["id"]+".epub");before=sha(path.read_bytes());patched=patch_epub_project_css(path,old_css,new_css)
            selected=ordered_rows(manifest,st);expected=sum(1 for r in selected for n in walk_nodes(ast_cache[r["id"]]) if n.get("t")=="Math")
            package=epub_helpers.validate_epub_package(path,expected)
            ec=subprocess.run([shutil.which("java") or "java","-jar",epubcheck,str(path)],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            if ec.returncode:raise RuntimeError(f"EPUBCheck 5.4.0 failed after CSS-only patch for {st['id']}: {ec.stdout[-2000:]}")
            rec={"artifact":path.name,"before_epub_sha256":before,**patched,"source_math_nodes":expected,"package_check":package,"epubcheck":{"version":"5.4.0","exit_code":ec.returncode,"output":ec.stdout[-1500:]}}
            records.append(rec)
        collection=out/"AllMight-Codex-Collection.epub";before=sha(collection.read_bytes());patched=patch_epub_project_css(collection,old_css,new_css)
        package=epub_helpers.validate_epub_package(collection,all_math)
        ec=subprocess.run([shutil.which("java") or "java","-jar",epubcheck,str(collection)],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        if ec.returncode:raise RuntimeError(f"EPUBCheck 5.4.0 failed after CSS-only patch for collection: {ec.stdout[-2000:]}")
        collection_rec={"artifact":collection.name,"before_epub_sha256":before,**patched,"source_math_nodes":all_math,"package_check":package,"epubcheck":{"version":"5.4.0","exit_code":ec.returncode,"output":ec.stdout[-1500:]}}
        report_path=out/"epub-build-report.json";report=json.loads(report_path.read_text(encoding="utf-8"))
        by_stage={r["stage_id"]:r for r in report["stage_epubs"]}
        for rec,st in zip(records,stages):
            by_stage[st["id"]].update({"sha256":rec["epub_sha256"],"bytes":rec["epub_bytes"],"package_check":rec["package_check"],"epubcheck":rec["epubcheck"],"css_patch":{"old_sha256":rec["old_css_sha256"],"new_sha256":rec["new_css_sha256"]}})
        report["collection_epub"].update({"sha256":collection_rec["epub_sha256"],"bytes":collection_rec["epub_bytes"],"package_check":collection_rec["package_check"],"epubcheck":collection_rec["epubcheck"],"css_patch":{"old_sha256":collection_rec["old_css_sha256"],"new_sha256":collection_rec["new_css_sha256"]}})
        report["browser_css_patch"]={"scope":"project CSS entry only; XHTML, OPF, MathML, code text, tables, links, CFI paths, and EPUB spine are unchanged","artifacts":records+[collection_rec]}
        report_path.write_text(json.dumps(report,indent=2,ensure_ascii=False),encoding="utf-8")
        print(f"CSS_PATCHED artifacts={len(records)+1} css_sha256={sha(new_css)} report={report_path}",flush=True)
        return 0
    if args.finalize_crossrefs_only:
        for st in stages: builder.compose("stage",st)
        paths={st["id"]:str(out/(st["id"]+".epub")) for st in stages}
        cfi_report=rewrite_cross_stage_cfis(paths,stages,builder.cross_stage_refs)
        summaries=[]
        for st in stages:
            epub=Path(paths[st["id"]]);selected=ordered_rows(manifest,st)
            expected=sum(1 for r in selected for n in walk_nodes(ast_cache[r["id"]]) if n.get("t")=="Math")
            package=epub_helpers.validate_epub_package(epub,expected)
            ec=subprocess.run([shutil.which("java") or "java","-jar","/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15/epubcheck/epubcheck-5.4.0/epubcheck.jar",str(epub)],text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
            if ec.returncode: raise RuntimeError(f"EPUBCheck 5.4.0 after CFI locator insertion failed for {st['id']}: {ec.stdout[-2500:]}")
            for ref in cfi_report[st["id"]]:
                resolved=resolve_epub_cfi(paths[ref["peer"]],ref["cfi"])
                expected_id=epub_actual_target_id(paths[ref["peer"]],ref["target"])
                if resolved!=expected_id: raise RuntimeError(f"CFI resolver mismatch: {ref['cfi']} -> {resolved}, expected {expected_id}")
                ref["resolved_id"]=resolved
            summaries.append({"stage_id":st["id"],"path":str(epub.resolve()),"sha256":sha(epub.read_bytes()),"bytes":epub.stat().st_size,"math_nodes":expected,"package_check":package,"epubcheck":{"version":"5.4.0","exit_code":ec.returncode,"output":ec.stdout[-2500:]},"cross_stage_cfi_count":len(cfi_report[st["id"]]),"cross_stage_cfi_links":cfi_report[st["id"]]})
        collection=out/"AllMight-Codex-Collection.epub"
        collection_package=epub_helpers.validate_epub_package(collection,all_math)
        status_data=json.loads((out/"build-status.json").read_text(encoding="utf-8"))
        collection_run=next((r for r in status_data.get("completed",[]) if r.get("artifact")==collection.name),None)
        if not collection_run or collection_run.get("epubcheck",{}).get("exit_code")!=0:
            raise RuntimeError("No successful EPUBCheck 5.4.0 record found for the current collection build")
        report={"generation_id":gen,"source_manifest":str((root/"conversion-manifest.json").resolve()),"stage_epubs":summaries,"collection_epub":{"path":str(collection.resolve()),"sha256":sha(collection.read_bytes()),"bytes":collection.stat().st_size,"math_nodes":all_math,"package_check":collection_package,"epubcheck":collection_run["epubcheck"]},"source_totals":{"rows":len(manifest["rows"]),"chapters":sum(r["kind"]=="chapter" for r in manifest["rows"]),"references":sum(r["kind"]=="reference" for r in manifest["rows"]),"math":all_math,"code_blocks":sum(1 for a in ast_cache.values() for n in walk_nodes(a) if n.get("t")=="CodeBlock"),"tables":sum(1 for a in ast_cache.values() for n in walk_nodes(a) if n.get("t")=="Table"),"unique_attachments":manifest.get("source_attachment_inventory",{}).get("unique_count")},"cross_stage_reference_note":"CFIs are stored in XHTML data attributes and this report; the visible stage book lists human locators and directs readers to the internally linked collection for offline jumps."}
        report_path=out/"epub-build-report.json";report_path.write_text(json.dumps(report,indent=2,ensure_ascii=False),encoding="utf-8")
        print(f"FINALIZED stage_epubs={len(summaries)} cross_stage_cfis={sum(s['cross_stage_cfi_count'] for s in summaries)} report={report_path}",flush=True)
        return 0
    if args.collection_only: stages=[]
    if args.stage:
        stages=[s for s in stages if s["id"]==args.stage]
        if not stages: ap.error(f"unknown stage: {args.stage}")
    reports=[];status=out/"build-status.json"
    for st in stages:
        report=builder.run_build(gen,st);reports.append(report)
        status.write_text(json.dumps({"generation_id":gen,"completed":reports,"pending_stages":[s["id"] for s in stages[len(reports):]],"assets":builder.asset_manifest},indent=2,ensure_ascii=False),encoding="utf-8")
        print(f"BUILT {report['artifact']} chapters={report['chapter_rows']} references={report['reference_rows']} attachments={report['source_attachments']} math={report['expected_math_nodes']} bytes={report['bytes']}",flush=True)
    if not args.stage or args.collection_only:
        collection=builder.run_build(gen,None);reports.append(collection)
        paths={st["id"]:str(out/(st["id"]+".epub")) for st in manifest["stages"]}
        cfi_report=rewrite_cross_stage_cfis(paths,manifest["stages"],builder.cross_stage_refs)
        for report in reports:
            if report.get("stage_id") in cfi_report:
                report["cross_stage_cfi_links"]=cfi_report[report["stage_id"]]
        status.write_text(json.dumps({"generation_id":gen,"completed":reports,"pending_stages":[],"assets":builder.asset_manifest},indent=2,ensure_ascii=False),encoding="utf-8")
        print(f"BUILT {collection['artifact']} chapters={collection['chapter_rows']} references={collection['reference_rows']} attachments={collection['source_attachments']} math={collection['expected_math_nodes']} bytes={collection['bytes']}",flush=True)
    return 0


if __name__=="__main__": raise SystemExit(main())
