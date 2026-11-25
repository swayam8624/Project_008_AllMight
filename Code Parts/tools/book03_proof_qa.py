#!/usr/bin/env python3
"""Inspect actual Book03 proof files, not just their input ASTs.

Checks chapter navigation, MathML counts, equation labels, code, embedded image
hashes, and sampled print geometry. The resulting contact sheets are for human
inspection; a green machine audit is not automatic reader acceptance.
"""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import posixpath
import re
import subprocess
from urllib.parse import unquote, urlsplit
import xml.etree.ElementTree as ET
import zipfile

from book03_prepare_reader import nodes, plain, require

NS = "{http://www.w3.org/1999/xhtml}"
MATH = "{http://www.w3.org/1998/Math/MathML}"
OPF = "{http://www.idpf.org/2007/opf}"


def sha(data):
    return hashlib.sha256(data).hexdigest()


def norm_code(s):
    # Permit CRLF conversion and an optional final newline, but preserve
    # indentation and internal whitespace, which may be semantically relevant.
    return s.replace("\r\n","\n").rstrip("\n")


def verify_epub(path, ast, receipt, approved_cover_sha256=None):
    with zipfile.ZipFile(path) as z:
        require(z.testzip() is None, "EPUB ZIP checksum")
        names = set(z.namelist())
        require(len(names)==len(z.namelist()), "duplicate ZIP members")
        require(z.namelist()[0]=="mimetype" and z.read("mimetype")==b"application/epub+zip",
                "invalid EPUB mimetype")
        require(z.getinfo("mimetype").compress_type==zipfile.ZIP_STORED, "compressed mimetype")
        container = ET.fromstring(z.read("META-INF/container.xml"))
        opf_name = next(el.attrib["full-path"] for el in container.iter()
                        if el.tag.endswith("}rootfile"))
        opf = ET.fromstring(z.read(opf_name))
        directory = posixpath.dirname(opf_name)
        manifest = {e.attrib["id"]: e.attrib for e in opf.iter(OPF+"item")}
        cover_items = [a for a in manifest.values() if "cover-image" in a.get("properties","").split()]
        nav = [a for a in manifest.values() if "nav" in a.get("properties","").split()]
        require(len(nav)==1, "exactly one navigation document required")

        def resolve(base, href):
            return posixpath.normpath(posixpath.join(base, unquote(urlsplit(href).path)))

        def location(item):
            return resolve(directory,item["href"])

        for item in manifest.values():
            require(location(item) in names, f"missing OPF resource: {item}")
        if approved_cover_sha256:
            require(len(cover_items)==1,"approved cover-image missing or ambiguous")
            require(sha(z.read(location(cover_items[0])))==approved_cover_sha256,
                    "EPUB approved cover bytes differ from staged image")
        spine = [manifest[e.attrib["idref"]] for e in opf.iter(OPF+"itemref")]
        bodies = [(location(a), ET.fromstring(z.read(location(a)))) for a in spine]
        docs = {name: ET.fromstring(z.read(name)) for name in names if name.endswith((".xhtml",".html"))}
        ids = {name:{el.attrib["id"] for el in doc.iter() if "id" in el.attrib}
               for name,doc in docs.items()}
        for name,doc in docs.items():
            for el in doc.iter():
                for attr in ("href","src"):
                    href = el.attrib.get(attr,"")
                    if not href or urlsplit(href).scheme or href.startswith("//"):
                        continue
                    u = urlsplit(href)
                    dest = resolve(posixpath.dirname(name),href) if u.path else name
                    require(dest in names, f"broken EPUB resource: {name}: {href}")
                    if u.fragment and dest in ids:
                        require(unquote(u.fragment) in ids[dest], f"broken EPUB anchor: {name}: {href}")
        navdoc = docs[location(nav[0])]
        nav_chapters = set()
        # Pandoc may link a level-one chapter directly to its XHTML file
        # instead of appending its heading ID. Resolve both representations.
        nav_name = location(nav[0])
        for el in navdoc.iter(NS+"a"):
            href = el.attrib.get("href","")
            u = urlsplit(href)
            m = re.fullmatch(r"chapter-(\d{2})",unquote(u.fragment))
            if m:
                nav_chapters.add(int(m[1]))
            elif u.path and not u.fragment and not u.scheme:
                dest = resolve(posixpath.dirname(nav_name),href)
                candidates = [re.fullmatch(r"chapter-(\d{2})",i) for i in ids.get(dest,set())]
                exact = [m for m in candidates if m]
                if len(exact)==1:
                    nav_chapters.add(int(exact[0][1]))
        require(nav_chapters==set(range(1,33)), f"navigation chapters incomplete: {nav_chapters}")
        math_count = sum(1 for _,d in bodies for _ in d.iter(MATH+"math"))
        expected_math = sum(1 for v in nodes(ast["blocks"]) if v["t"]=="Math")
        require(math_count==expected_math, f"MathML count {math_count} != {expected_math}")
        expected_labels = [plain(v["c"][1]) for v in nodes(ast["blocks"])
                           if v["t"]=="Span" and "eq-label" in v["c"][0][1]]
        labels = ["".join(e.itertext()) for _,d in bodies for e in d.iter(NS+"span")
                  if "eq-label" in e.attrib.get("class","").split()]
        require(Counter(labels)==Counter(expected_labels),"equation labels lost or changed")
        expected_code = [norm_code(v["c"][1]) for v in nodes(ast["blocks"]) if v["t"]=="CodeBlock"]
        code = [norm_code("".join(e.itertext())) for _,d in bodies for e in d.iter(NS+"pre")]
        require(code==expected_code,"EPUB code block order or text differs from shared AST")
        expected_images = {v["png_sha256"] for v in receipt["figures"].values()}
        embedded = {sha(z.read(location(a))) for a in manifest.values()
                    if a.get("media-type")=="image/png"}
        require(expected_images <= embedded,"one or more staged figure PNGs absent from EPUB")
        index_links = [e for _,d in bodies for e in d.iter(NS+"a") if "index-reference" in e.attrib.get("class","").split()]
        expected_index = sum(1 for v in nodes(ast["blocks"]) if v["t"]=="Link" and "index-reference" in v["c"][0][1])
        require(expected_index>0 and len(index_links)==expected_index,"subject index links missing")
        return {"subject_index_links":len(index_links),"chapter_navigation":len(nav_chapters),"mathml":math_count,
                "equation_labels":len(labels),"code_blocks":len(code),
                "figure_hashes_matched":len(expected_images),"spine_documents":len(spine)}


def verify_pdf(path, ast, out, has_cover=False):
    from PIL import Image, ImageDraw
    def command(args):
        return subprocess.run(args,check=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True).stdout

    info = command(["pdfinfo",str(path)])
    pages = int(re.search(r"(?m)^Pages:\s+(\d+)",info)[1])
    size = re.search(r"(?m)^Page size:\s+([\d.]+) x ([\d.]+) pts",info)
    require(size and abs(float(size[1])-576)<.1 and abs(float(size[2])-720)<.1,"unexpected print page dimensions")
    if has_cover:
        images = command(["pdfimages","-f","1","-l","1","-list",str(path)])
        require(re.search(r"(?m)^\s*1\s+\d+\s+image\s+",images),"approved print cover missing on first page")
    raw = command(["pdftotext","-layout",str(path),"-"])
    # Compare extracted titles only at the destinations recorded by LaTeX.
    page_text = [" ".join(p.split()) for p in raw.split("\f")[:pages]]
    titles = [(v["c"][1][0],plain(v["c"][2])) for v in nodes(ast["blocks"])
              if v["t"]=="Header" and re.fullmatch(r"chapter-\d{2}",v["c"][1][0])]
    require(len(titles)==32,"print AST chapter count")
    starts = {}
    aux = (out/"reader-proof.aux").read_text(encoding="utf-8")
    labels = dict(re.findall(r"(?m)^\\newlabel\{(chapter-\d{2})\}.*\{(chapter\*?\.\d+)\}\{\}\}$", aux))
    destinations = {
        name: int(page)
        for page, name in re.findall(r'(?m)^\s*(\d+)\s+\[[^\n]*\]\s+"([^"]+)"', command(["pdfinfo","-dests",str(path)]))
    }
    for anchor,title in titles:
        page = destinations.get(labels.get(anchor))
        require(page and 1 <= page <= pages, f"missing PDF destination for {anchor}")
        key = re.sub(r"[^a-z0-9]","",title.casefold())
        require(key in re.sub(r"[^a-z0-9]","",page_text[page-1].casefold()),
                f"missing extracted chapter title at PDF destination: {title}")
        require(not starts or page > max(starts.values()), f"out-of-order PDF chapter: {anchor}")
        starts[anchor] = page
    sparse = [i+1 for i,p in enumerate(page_text) if i+1 not in (1,2,3) and len(p.split())<55]
    selected=sorted(set([1,min(2,pages),pages]+sparse+list(starts.values())+
                        [page-1 for page in list(starts.values())[1:]]+
                        [max(1,min(pages,round(pages*f/10))) for f in range(1,10)]))
    # Every opening plus distributed interiors. This is a sample, not an
    # exhaustive clipping/glyph or mathematical correctness certificate.
    thumbdir=out/"pdf-page-samples"
    thumbdir.mkdir()
    thumbs=[]
    for n in selected:
        prefix=thumbdir/f"page-{n:04d}"
        subprocess.run(["pdftoppm","-f",str(n),"-l",str(n),"-r","95","-png","-singlefile",
                        str(path),str(prefix)],check=True,stdout=subprocess.DEVNULL)
        with Image.open(prefix.with_suffix(".png")) as im:
            rgb=im.convert("RGB")
            rgb.thumbnail((280,350))
            thumbs.append((n,rgb.copy()))
    for start in range(0,len(thumbs),12):
        sheet=Image.new("RGB",(1200,3*390),"#e8edf0")
        d=ImageDraw.Draw(sheet)
        for j,(n,im) in enumerate(thumbs[start:start+12]):
            x,y=10+(j%4)*300,10+(j//4)*390
            sheet.paste(im,(x+(280-im.width)//2,y))
            d.text((x,y+354),f"PDF page {n}",fill="#183344")
        sheet.save(out/f"pdf-contact-{start//12+1:02d}.png")
    require("Subject index" in raw,"print subject index missing")
    return {"sparse_text_pages_for_review":sparse,"pages":pages,"page_size_pt":[float(size[1]),float(size[2])],
            "chapter_title_pages":starts,"sampled_pages":selected}


def scan_pdf_geometry(path, expected_figures, has_cover):
    """Inspect the actual finished PDF using PyMuPDF, without rasterizing all pages."""
    import fitz
    import collections
    histogram = collections.Counter()
    empty, overflow = [], []
    images = set()
    with fitz.open(path) as doc:
        require(not doc.is_repaired, "PDF needed structural repair during opening")
        for page in doc:
            require(abs(page.rect.width-576)<.1 and abs(page.rect.height-720)<.1,
                    f"unexpected PDF trim on page {page.number+1}")
            info = page.get_image_info(hashes=True)
            for im in info:
                images.add(im["digest"])
            text = page.get_text("dict", flags=fitz.TEXTFLAGS_DICT & ~fitz.TEXT_PRESERVE_IMAGES,
                                 clip=fitz.INFINITE_RECT())
            blocks = [b for b in text["blocks"] if b.get("type")==0]
            if not info and not any(s["text"].strip() for b in blocks for line in b["lines"] for s in line["spans"]):
                empty.append(page.number+1)
            for bounds in [b["bbox"] for b in blocks] + [im["bbox"] for im in info]:
                x0,y0,x1,y1=bounds
                if min(x0,y0)<-0.75 or x1>page.rect.width+0.75 or y1>page.rect.height+0.75:
                    overflow.append({"page":page.number+1,"bbox":list(bounds)})
            for b in blocks:
                for line in b["lines"]:
                    for s in line["spans"]:
                        font=re.sub(r"[^a-z]","",s["font"].casefold())
                        if "dejavusansmono" in font:
                            histogram[round(s["size"],2)] += len(s["text"].strip())
        outline = doc.get_toc()
        chapters={int(m[1]) for _,title,_ in outline if (m:=re.match(r"Chapter (\d{2})\b",title))}
        require(chapters==set(range(1,33)),"PDF chapter bookmarks incomplete")
    require(not empty,f"blank PDF pages: {empty}")
    require(not overflow,f"out-of-page PDF content: {overflow[:12]}")
    require(len(images)>=expected_figures+int(has_cover),"PDF illustration objects missing")
    total=sum(histogram.values())
    in_range=sum(count for size,count in histogram.items() if 8.0<=size<=8.6)
    require(total and in_range/total>=.90,
            f"effective monospaced size out of range: {dict(histogram)}")
    return {"blank_pages":empty,"out_of_page_blocks":overflow,
            "unique_embedded_images":len(images),"chapter_bookmarks":len(chapters),
            "effective_monospace_pdf_pt":dict(histogram),
            "monospace_characters_in_8_to_8_6_pt":in_range/total}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("proof",type=Path)
    p.add_argument("--strict-pdf",action="store_true",help="require PyMuPDF and scan every finished PDF page")
    a=p.parse_args()
    out=a.proof.resolve()
    receipt=json.loads((out/"source-proof.json").read_text())
    print_ast=json.loads((out/"reader-print.json").read_text())
    ebook_ast=json.loads((out/"reader-epub.json").read_text())
    build=json.loads((out/"build-proof.json").read_text())
    cover=build.get("approved_cover_sha256")
    require(build.get("epubcheck_version")=="5.4.0", "formal validator receipt absent")
    result={"epub":verify_epub(out/"Book03-From-Geometry-to-Images-PROOF.epub",ebook_ast,receipt,cover),
            "pdf":verify_pdf(out/"Book03-From-Geometry-to-Images-PROOF.pdf",print_ast,out,bool(cover)),
            "accepted":False,
            "visual_review":"pending human review of samples and all publication pages"}
    if a.strict_pdf:
        result["pdf"]["whole_file_scan"]=scan_pdf_geometry(out/"Book03-From-Geometry-to-Images-PROOF.pdf",len(receipt["figures"]),bool(cover))
    (out/"format-audit.json").write_text(json.dumps(result,indent=2)+"\n")
    print(json.dumps(result,indent=2))


if __name__=="__main__":
    main()
