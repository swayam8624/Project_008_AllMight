#!/usr/bin/env python3
"""Additive reader-publication finishing pass for Book Two EPUB 3.

Converts a verified complete ebook into a reader-friendly copy without rewriting
its frozen mathematical manuscript. It splits oversized XHTML safely, repairs
navigation/metadata, introduces front/back study aids, adds numbered scholarly
figures and selected equations, optimizes large SVG artwork, and verifies
existing mathematical and code payloads survived the packaging step.

Dependencies: lxml, Pillow, cairosvg (the last is used only for large SVGs).
No changes to the input EPUB; all outputs are fresh files.
"""
from __future__ import annotations
import argparse, collections, copy, datetime, hashlib, html, io, json, os, pathlib, posixpath, re, sys, unicodedata, urllib.parse, zipfile
from lxml import etree as E
from PIL import Image

NS={'x':'http://www.w3.org/1999/xhtml','m':'http://www.w3.org/1998/Math/MathML','opf':'http://www.idpf.org/2007/opf','dc':'http://purl.org/dc/elements/1.1/','epub':'http://www.idpf.org/2007/ops','ncx':'http://www.daisy.org/z3986/2005/ncx/'}
X=NS['x'];M=NS['m'];OPF=NS['opf'];DC=NS['dc'];EPUB=NS['epub'];NCX=NS['ncx']
Q=lambda ns,tag:'{%s}%s'%(ns,tag)

P='EPUB/'
OUTFILE='From Equations to Geometry - Publication Revision.epub'
SOURCE_REPO='https://github.com/swayam8624/Project_008_AllMight'
BRANCH='curriculum/book02-mathematics-refinement-pass01'

TERMS=[
 'absolute error','affine transform','aliasing','angle-axis','arithmetic mean','automatic differentiation','Bayesian inference','barycentric coordinates',
 'bias','binary64','bisection','Bezier','Bernstein','bivector','broadcasting','cancellation','camera projection',
 'Cauchy','Cholesky','condition number','conditioning','confidence interval','conjugate gradient','continuity','convergence','correlation',
 'covariance','cross product','cubic spline','determinant','differential equation','discretization','distribution','dot product',
 'eigenvalue','eigenvector','Euler method','expectation','finite difference','finite element','floating point','Fresnel','Gauss-Newton',
 'geodesic','gradient','Gram-Schmidt','Hamiltonian','heat equation','Hermite','Hessian','homogeneous coordinates','importance sampling',
 'integration','Jacobian','Kalman','least squares','linear algebra','linear transformation','LU factorization','mass matrix',
 'matrix exponential','Monte Carlo','Newton method','normal matrix','numerical stability','orthogonal','outlier',
 'Poisson','probability','projection','pseudoinverse','QR decomposition','quadrature','quaternion','RANSAC','recursion',
 'refraction','reflection','residual','rigid body','rotation','sampling','sensitivity','Simpson','singular value decomposition',
 'singular values','spline','stability','stiffness','Taylor','time stepping','trapezoid','variance','vector','von Mises',
]

GLOSSARY={
 'Affine transformation':'Map combining a linear transformation and translation; it preserves lines and parallelism.',
 'Barycentric coordinates':'Weights relative to triangle vertices that sum to one and reconstruct a point in their affine span.',
 'Condition number':'Measure of how sensitive the solution of a specified problem is to input perturbations.',
 'Covariance':'Expected product of centered random variables; sign and scale depend on chosen variables and units.',
 'Determinant':'Signed volume-scaling factor of a square linear transformation; zero means singular.',
 'Eigenvector':'Nonzero direction changed only by a scalar under a linear transformation.',
 'Finite difference':'Approximation of a derivative using values at neighboring points and a specified step.',
 'Gradient':'Vector representing the derivative of a scalar field under the stated inner product and coordinate convention.',
 'Heat equation':'Partial differential equation modeling diffusion as a time change proportional to a spatial Laplacian.',
 'Hessian':'Matrix of second derivatives (under regularity assumptions); describes local curvature of a scalar objective.',
 'Importance sampling':'Monte Carlo technique changing the proposal distribution with compensating probability weights.',
 'Jacobian':'Linear map of first derivatives of a vector-valued function at an evaluation point.',
 'Least squares':'Estimation minimizing a stated sum of squared residuals, not automatically robust to outliers.',
 'MathML':'Semantic XML vocabulary used here for equations and mathematical expressions in EPUB 3.',
 'Monte Carlo':'Numerical estimation using samples from a specified probability law, with assumptions governing variance.',
 'Normal matrix':'For surface normals under an invertible affine transform, the inverse-transpose of the linear part.',
 'Numerical stability':'Property of an algorithm describing how it controls rounding and other computational errors.',
 'Orthogonal projection':'Map onto a subspace whose error is perpendicular to the subspace.',
 'Pseudoinverse':'Generalized inverse that defines a least-squares, minimum-norm solution under specified conditions.',
 'Quaternion':'Four-component algebraic representation often used for composing and interpolating 3D rotations.',
 'QR decomposition':'Factorization of a matrix into an orthogonal/unitary factor and an upper-triangular factor.',
 'RANSAC':'Sampling-based method that forms candidate models and evaluates their agreement with observed data.',
 'Refraction':'Direction change of light between media according to the stated refractive-index model.',
 'Residual':'Difference between a model prediction and a measured or prescribed observation.',
 'Singular value decomposition':'Factorization of a matrix into orthogonal/unitary factors and nonnegative singular-value scaling.',
 'Variance':'Expected squared deviation from the mean, assuming it exists.',
}

SYMBOLS=[
 ('x, y, z','Coordinates or scalar variables; meaning depends on the chapter and coordinate frame.'),
 ('v, n','Vector and surface normal; length and coordinate convention must be stated.'),
 ('A, M, K','Generic matrix, mass matrix, and stiffness matrix, respectively, where defined.'),
 ('A⁻¹, Aᵀ','Inverse (if it exists) and transpose of a matrix.'),
 ('‖x‖','Norm of x under the specified inner product.'),
 ('∇f','Gradient of scalar field f, evaluated at a stated point.'),
 ('J','Jacobian matrix of the function being differentiated.'),
 ('H','Hessian or other chapter-defined matrix; check local definitions.'),
 ('Σ, μ','Covariance matrix and statistical mean.'),
 ('E[X], Var(X)','Expectation and variance of a random variable.'),
 ('Δt, h','Time step and spatial discretization step, according to context.'),
 ('ε','Small error/tolerance; never assume a universal numeric value.'),
 ('p(x), q(x)','Target and proposal densities in sampling; support and normalization matter.'),
 ('R, t','Rotation and translation in a geometric transform.'),
 ('φᵢ','Basis function in a finite-element approximation.'),
]

PREREQ={
1:'Basic Python syntax is useful here but not required for the later C++23 mathematics. Expect familiarity with arithmetic, units, and reading code.',
2:'Arithmetic with signed numbers, Pythagoras, basic 2D vectors, and the difference between points and directions.',
3:'Vectors, dot products, bases, linear maps, and awareness that coordinates depend on a chosen frame.',
4:'Matrix multiplication, solving small linear systems, inverse and transpose, and orthogonality.',
5:'Functions of several variables, vectors, matrix multiplication, first derivatives, and coordinate frames.',
6:'One-variable derivatives, antiderivatives, integrals, and basic approximation/error vocabulary.',
7:'Fractions, sets, integrals, and the distinction between an observed frequency and a modeled probability.',
8:'Expectation, variance, covariance, inner products, and eigenvectors.',
9:'Elementary derivatives, linear ODEs, vector spaces, and matrix operations.',
10:'Matrix calculus, Jacobians, least-squares objectives, and numerical conditioning.',
11:'Differentials, 3D coordinate frames, least-squares residuals, and matrix factorizations.',
12:'Derivatives, root finding, optimization objectives, and floating-point error concepts.',
13:'Finite differences, Taylor expansions, numerical integration, and error measurement.',
14:'Probability distributions, expectations, independent samples, and variance.',
15:'Sampling distributions, probability density, random generators, and numerical contracts.',
16:'Probability, covariance, Monte Carlo estimation, confidence statements, and experiment design.',
}

def parse(data):
    return E.fromstring(data,parser=E.XMLParser(remove_blank_text=False,resolve_entities=False,no_network=True,huge_tree=True))

def ser(root,doctype=None):
    return E.tostring(root,encoding='UTF-8',xml_declaration=True,doctype=doctype)

def el(tag,text=None,**attrs):
    a=E.Element(Q(X,tag),**attrs)
    if text is not None:a.text=text
    return a

def inside(root,tag):
    return root.xpath('.//x:'+tag,namespaces=NS)

def local(item):return E.QName(item).localname if isinstance(item.tag,str) else ''

def norm(t):return re.sub(r'\s+',' ',t or '').strip()

def body(root):return root.find(Q(X,'body'))

def fresh_page(title,body_nodes,chapter=None):
    root=E.Element(Q(X,'html'),nsmap={None:X,'epub':EPUB})
    root.set('lang','en');root.set('{http://www.w3.org/XML/1998/namespace}lang','en')
    head=E.SubElement(root,Q(X,'head'))
    E.SubElement(head,Q(X,'title')).text=title
    E.SubElement(head,Q(X,'link'),rel='stylesheet',type='text/css',href='../styles/stylesheet1.css')
    b=E.SubElement(root,Q(X,'body'))
    for node in body_nodes:b.append(node)
    return root

def section(title,identifier,level='h1'):
    sec=el('section',id=identifier)
    sec.append(el(level,title))
    return sec

def ptext(text,style=None):
    p=el('p',text)
    if style:p.set('class',style)
    return p

def index_of_parent(root,node):
    par=node.getparent();return par,par.index(node)

class Editor:
    def __init__(self,source):
        self.source=pathlib.Path(source)
        with zipfile.ZipFile(self.source) as z:
            self.data={name:z.read(name) for name in z.namelist()}
            self.originals={x:copy.copy(z.getinfo(x)) for x in z.namelist()}
            assert z.testzip() is None
        self.names=list(self.data)
        self.opf=parse(self.data['EPUB/content.opf'])
        self.manifest=self.opf.find(Q(OPF,'manifest'))
        self.spine=self.opf.find(Q(OPF,'spine'))
        self.items={a.get('id'):a for a in self.manifest}
        self.href_item={a.get('href'):a for a in self.manifest}
        self.changed=set()
        self.reading_order=[2,3,4,1]+list(range(5,17))  # Original Python tools chapter after matrices.
        self.chapter_paths={}
        self.chapter_titles={}
        self.chapter_roots={}
        self.ids={}
        self.figure_records=[]
        self.equation_records=[]
        self.math_before=self._count_tag('math')
        self.chapter_math_before=sum(len(parse(self.data[f'EPUB/text/ch{n:03}.xhtml']).xpath('.//m:math',namespaces=NS)) for n in range(6,22))
        self.code_before=self._count_tag('pre')
        self.fig_before=self._count_tag('img')
        self.chapter_fig_before=sum(len(parse(self.data[f'EPUB/text/ch{n:03}.xhtml']).xpath('.//x:img',namespaces=NS)) for n in range(6,22))
        self.report={}
    def _count_tag(self,t):
        count=0
        for name,d in self.data.items():
            if name.endswith('.xhtml') and name.startswith('EPUB/text/'):
                try:count+=len(parse(d).xpath('.//*[local-name()=$tag]',tag=t))
                except E.XMLSyntaxError:pass
        return count
    def set(self,name,data):
        if isinstance(data,E._Element):data=ser(data)
        if name not in self.data:self.names.append(name)
        self.data[name]=data;self.changed.add(name)
    def drop(self,name):
        if name in self.data:del self.data[name]
        if name in self.names:self.names.remove(name)
        self.changed.add(name)
    def add_item(self,id,href,mime,properties=None,spine=True):
        if id in self.items or href in self.href_item:
            raise ValueError('Already declared EPUB resource '+id+' '+href)
        attrs={'id':id,'href':href,'media-type':mime}
        if properties:attrs['properties']=properties
        item=E.SubElement(self.manifest,Q(OPF,'item'),**attrs)
        self.items[id]=item;self.href_item[href]=item
        if spine:E.SubElement(self.spine,Q(OPF,'itemref'),idref=id)
        return item
    def get_root(self,name):return parse(self.data[name])
    def _first_math(self,block):
        if not isinstance(block.tag,str):return None
        found=[x for x in block.iter() if isinstance(x.tag,str) and x.tag==Q(M,'math') and x.get('display')=='block']
        return found[0] if found else None
    def _chapter_nodes(self,number,root):
        bb=body(root)
        if len(bb)!=1 or local(bb[0])!='section':raise ValueError('Unexpected chapter structure')
        book=bb[0];root_id=book.get('id')
        out=[]
        def recurse(parent):
            for child in list(parent):
                if not isinstance(child.tag,str):continue
                if local(child)=='section':
                    if child.get('id'):
                        out.append(el('span',id=child.get('id'),**{'class':'section-anchor'}))
                    recurse(child)
                else:
                    out.append(copy.deepcopy(child))
        recurse(book)
        return root_id,out
    def rewrite_chapters(self,chunk_target=45000,chunk_hard=52000):
        """Split intact section-nested chapters into 30-62KB XHTML fragments."""
        documents=[]
        for c in range(1,17):
            original_n=self.reading_order[c-1]
            old=f'EPUB/text/ch{original_n+5:03}.xhtml'
            root=self.get_root(old)
            chapter_root_id,nodes=self._chapter_nodes(c,root)
            h1=next((a for a in nodes if local(a)=='h1'),None)
            if h1 is None:raise ValueError('Missing chapter heading '+old)
            title=norm(''.join(h1.itertext()));self.chapter_titles[c]=title
            chapter_num=el('span',f'Chapter {c} · ',**{'class':'chapter-number'})
            chapter_num.tail=h1.text;h1.text=None;h1.insert(0,chapter_num)
            # Set the prerequisites before any original lesson, after h1.
            box=el('aside',**{'class':'prerequisite-box','id':f'prerequisites-{c}'})
            box.append(el('strong','Prerequisites · '))
            q=el('span',PREREQ[original_n]);box.append(q)
            nodes.insert(nodes.index(h1)+1,box)
            # Add one concise proof/check result only in relevant chapters.
            if original_n in (2,4,10,14):
                proof=self._key_result(original_n)
                nodes.insert(min(9,len(nodes)),proof)
            # Number one meaningful display equation per large section.
            eq_no=0;await_first=False
            for block in nodes:
                if local(block)=='h2':await_first=True
                elif await_first:
                    math=self._first_math(block)
                    if math is not None:
                        eq_no+=1;eid=f'book02-eq-{c}-{eq_no}'
                        math.set('id',eid)
                        par,index=index_of_parent(root,math) if False else (math.getparent(),None)
                        if par is not None:
                            idx=par.index(math)
                            span=el('span',f'({c}.{eq_no})',**{'class':'equation-number','id':eid+'-label'})
                            tail=math.tail;math.tail=' ';span.tail=tail
                            par.insert(idx+1,span)
                        self.equation_records.append((c,eq_no,eid))
                        await_first=False
            # Construct bounded pieces from safe block nodes, preferring h2 boundaries.
            groups=[];cur=[];size=0
            for node in nodes:
                estimate=len(E.tostring(node,encoding='UTF-8'))
                if cur and ((size+estimate>chunk_target and size>=28000 and local(node)=='h2')
                             or size+estimate>chunk_hard):
                    groups.append(cur);cur=[];size=0
                cur.append(node);size+=estimate
            if cur:groups.append(cur)
            paths=[]
            for i,g in enumerate(groups,1):
                path=(old if i==1 else old.replace('.xhtml',f'-p{i:02}.xhtml'))
                new_root=copy.deepcopy(root)
                new_body=body(new_root)
                for child in list(new_body):new_body.remove(child)
                if i==1:
                    sec=el('section',id=chapter_root_id,**{'class':'level1'})
                else:
                    sec=el('section',**{'class':'chapter-continuation'})
                for node in g:sec.append(node)
                new_body.append(sec)
                # Do not repeat 'Chapter N' h1 across fragments.
                head=new_root.find(Q(X,'head'))
                if head is not None:
                    title_el=head.find(Q(X,'title'))
                    if title_el is not None:title_el.text=f'{title} — Part {i}' if i>1 else title
                self.set(path,new_root)
                if i>1:
                    old_item=self.href_item.get(old[len('EPUB/'):])
                    prop=(old_item.get('properties') or '') if old_item is not None else ''
                    self.add_item(f'ch{c+5:03}_part{i:02}',path[len('EPUB/'):],
                                  'application/xhtml+xml',prop,spine=False)
                paths.append(path)
            self.chapter_paths[c]=paths
            self.chapter_roots[c]=chapter_root_id
            documents.extend(paths)
        # Rebuild the spine in *reader order*: begin with the mathematical
        # hook (vectors, coordinate frames, factorizations), teach Python as
        # the supporting data/plotting tool in Chapter 4, then the remaining
        # 12 mathematical chapters. All original 16 source documents remain.
        original_refs=list(self.spine)
        lookup={r.get('idref'):r for r in original_refs}
        chapter_items=[]
        for c in range(1,17):
            for path in self.chapter_paths[c]:
                item=self.href_item[path[len('EPUB/'):]]
                ident=item.get('id')
                chapter_items.append(lookup.get(ident,E.Element(Q(OPF,'itemref'),idref=ident)))
        exclude={r.get('idref') for r in chapter_items}
        first=min(i for i,r in enumerate(original_refs) if r.get('idref') in exclude)
        front=[r for r in original_refs[:first] if r.get('idref') not in exclude]
        others=[r for r in original_refs[first:] if r.get('idref') not in exclude]
        for item in list(self.spine):self.spine.remove(item)
        for item in front+chapter_items+others:self.spine.append(item)
        self.report['chapter_reading_order']=[self.chapter_titles[c] for c in range(1,17)]
        self.report['chapters_split']={str(c):len(self.chapter_paths[c]) for c in range(1,17)}
        self.report['xhtml_max_kib_pre_edits']=round(max(len(self.data[p]) for p in documents)/1024,1)
    def _key_result(self,c):
        content={
          2:('Reflection as an orthogonal decomposition',
             'Assume a unit surface normal n and a direction v. Split v into tangent and normal parts. Reversing the normal part yields r = v − 2(v·n)n. The derivation requires ‖n‖ = 1; if the normal is not normalized, divide the projection by n·n.'),
          4:('Positive-definite Cholesky witness',
             'If a real symmetric matrix A has a Cholesky factorization A = LLᵀ with nonsingular L, then xᵀAx = ‖Lᵀx‖² > 0 for every nonzero x. Conversely, a zero or negative pivot in an attempted Cholesky factorization warns that the assumptions or floating-point policy need inspection.'),
          10:('Gauss–Newton step and its limitation',
              'Linearize residuals r(θ + Δ) ≈ r(θ) + JΔ. The least-squares stationarity equation is JᵀJΔ = −Jᵀr, assuming differentiability. Forming JᵀJ squares the condition number; use QR or SVD when conditioning makes the normal equations unreliable.'),
          14:('The one-over-square-root-N sampling law',
              'Let X₁,…,Xₙ be independent, identically distributed samples with finite variance σ². The sample mean has variance σ²/N, so its standard deviation is σ/√N. Correlated samples, infinite variance, or biased sampling without correct weights invalidate this formula.'),
        }
        title,description=content[c]
        a=el('aside',**{'class':'book02-theorem','id':f'book02-key-result-{c}'})
        a.append(el('strong','Key result and derivation · '+title))
        a.append(ptext(description))
        return a
    def chapter_parts(self):
        for n in range(1,17):
            for path in self.chapter_paths[n]:yield n,path
    def add_captions_and_math_access(self):
        fig_count=collections.defaultdict(int)
        math_count=0
        for n,path in self.chapter_parts():
            root=self.get_root(path)
            b=body(root)
            for math in list(b.iter(Q(M,'math'))):
                if not math.get('alttext'):
                    readable=norm(''.join(math.itertext()))
                    if readable:math.set('alttext',readable[:260])
                math_count+=1
            imgs=list(b.iter(Q(X,'img')))
            for image in imgs:
                fig_count[n]+=1
                figure_no=fig_count[n]
                ident=f'book02-fig-{n}-{figure_no}'
                image.set('id',ident)
                desc=norm(image.get('alt'))
                if not desc:
                    raise RuntimeError('Figure has no useful alternative description '+path)
                # Place caption after complete containing block, never inside an
                # anchor or a MathML expression. Original picture stays intact.
                target=image
                parent=target.getparent()
                while parent is not None and local(parent) not in ('figure','p','div','section'):
                    target=parent;parent=target.getparent()
                if parent is None:raise RuntimeError('Uncontained image '+ident)
                caption_text=desc[:190].rstrip('.,; ')+('.' if not desc[:190].endswith('.') else '')
                if local(parent)=='figure':
                    existing=[x for x in parent if local(x)=='figcaption']
                    if existing:
                        old=norm(''.join(existing[0].itertext()))
                        existing[0].text=f'Figure {n}.{figure_no} — '+(old or caption_text)
                    else:
                        parent.append(el('figcaption',f'Figure {n}.{figure_no} — {caption_text}'))
                else:
                    outer=parent
                    for cand in (image,parent):
                        if local(cand)=='p':outer=cand;break
                    dest=outer.getparent()
                    if dest is None:raise RuntimeError('Could not append caption '+ident)
                    caption=el('p',f'Figure {n}.{figure_no} — {caption_text}',**{'class':'book02-figure-caption'})
                    dest.insert(dest.index(outer)+1,caption)
                self.figure_records.append({'chapter':n,'number':figure_no,
                                            'id':ident,'file':path,'alt':desc})
            self.set(path,root)
        self.report['figure_captions']=sum(fig_count.values())
        self.report['mathml_alttext_checked']=math_count
        self.report['equations_numbered']=len(self.equation_records)
        if math_count!=self.chapter_math_before:
            raise RuntimeError(f'MathML node loss in 16 chapters: original={self.chapter_math_before} final={math_count}')
        if self.report['figure_captions']!=self.chapter_fig_before:
            raise RuntimeError('Scientific images dropped during chapter splitting')
    def optimize_rich_svg(self):
        import cairosvg
        optimized=[]
        # These are expensive path-heavy scientific Matplotlib SVGs. The
        # existing high-resolution vectors remain in the book PDF. For EPUB
        # reader reliability generate transparent PNG previews at 1600 px.
        targets=['file40.svg','file41.svg','file39.svg','file34.svg']
        for name in targets:
            source='EPUB/media/'+name
            if source not in self.data:continue
            original=self.data[source]
            if len(original)<1_000_000:continue
            new='EPUB/media/'+name.replace('.svg','-reader.png')
            try:
                rendered=cairosvg.svg2png(bytestring=original,output_width=1600)
                with Image.open(io.BytesIO(rendered)) as img:
                    if img.width!=1600 or img.height<120 or img.height>1500:
                        raise RuntimeError('Bad SVG preview dimensions')
                    img.verify()
            except Exception as ex:
                # A failed conversion is a documented limitation, never an
                # excuse to silently delete or replace the scientific figure.
                print('SVG_CONVERSION_WARNING',name,str(ex),file=sys.stderr)
                continue
            item=None
            for candidate in self.manifest:
                if candidate.get('href')=='media/'+name:item=candidate;break
            if item is None:raise RuntimeError('SVG exists but missing OPF '+name)
            old_href=item.get('href')
            item.set('href','media/'+name.replace('.svg','-reader.png'))
            item.set('media-type','image/png')
            self.href_item.pop(old_href,None)
            self.href_item[item.get('href')]=item
            self.set(new,rendered)
            self.drop(source)
            optimized.append({'from':name,'to':new.rsplit('/',1)[-1],
                              'svg_uncompressed_bytes':len(original),
                              'png_bytes':len(rendered)})
            # All references to the old asset must change; internal rich
            # scientific charts remain the same plotted pixel data.
            old_bytes=name.encode('utf8');new_bytes=new.rsplit('/',1)[-1].encode('utf8')
            for x in list(self.data):
                if x.endswith(('.xhtml','.css','.ncx')):
                    data=self.data[x]
                    if old_bytes in data:self.set(x,data.replace(old_bytes,new_bytes))
        self.report['rich_svgs_converted_to_png']=optimized
    def fix_css(self):
        css_path='EPUB/styles/stylesheet1.css'
        css=self.data[css_path].decode('utf-8')
        css=css.replace(':root { color-scheme: light; }','')
        css=css.replace('body { box-sizing: border-box; width: 100%; max-width: 80ch; margin: 0 auto; padding: 1rem; overflow-wrap: anywhere; font-family: Palatino, "Palatino Linotype", "Book Antiqua", Georgia, serif; line-height: 1.55; color: #202020; }',
            'body { box-sizing:border-box; width:auto; max-width:none; margin:0; padding:0; overflow-wrap:break-word; font-family:inherit; line-height:1.48; hyphens:auto; -webkit-hyphens:auto; }')
        css=css.replace('h1, h2, h3, h4 { font-family: Palatino, Baskerville, Georgia, serif; font-weight: 600; line-height: 1.2; color: #111; }',
            'h1, h2, h3, h4, h5 { font-family:inherit; font-weight:600; line-height:1.24; break-after:avoid; page-break-after:avoid; }')
        css=css.replace('a, a:visited { color: #7a2e2e; text-decoration: underline; }',
            'a, a:visited { text-decoration:underline; }')
        css=css.replace('img, figure, svg { max-width: 100%; height: auto; }',
            'img, figure, svg { max-width:100%; height:auto; }')
        css=css.replace('table { display: block; width: max-content; max-width: 100%; overflow-x: auto; border-collapse: collapse; }',
            'table { width:100%; max-width:100%; border-collapse:collapse; overflow-wrap:anywhere; }')
        css=css.replace('    background: transparent; color: inherit;','    background: #172e39; color: #f4f7f9;')
        css=css.replace('  div.Shaded, pre { background: transparent; }',
            '  div.booktwoproof {background:#292439;color:#f7f2fa;}\n'
            '  div.booktwopitfall {background:#32291d;color:#fcf1e2;}\n'
            '  div.booktwolab {background:#17312e;color:#e6f7f5;}\n'
            '  div.Shaded, pre { background:#20272e; color:#eaf0f5; }')
        css+='''
/* Book Two one-volume reader edition: user-controlled fonts and margins. */
html { margin:0; padding:0; }
body { widows:2; orphans:2; }
h1,h2,h3,h4,h5,h6 { break-after:avoid; page-break-after:avoid; hyphens:none; }
.chapter-number { font-size:.74em; font-weight:500; letter-spacing:.02em; }
.chapter-continuation { margin:0; padding:0; }
.section-anchor { display:block; height:0; font-size:0; }
.prerequisite-box,.book02-theorem { margin:1em 0; padding:.7em .85em;
 background:#ecf3f5; border-left:.25em solid #3a788b; break-inside:avoid; }
.book02-theorem { background:#f0ecf8; border-left-color:#7664a7; }
.equation-number { float:right; font-size:.78em; font-style:normal;
 color:inherit; opacity:.8; margin-left:.4em; }
.book02-figure-caption,figcaption { display:block; margin:.45em 0 1.25em;
 font-size:.87em; font-style:italic; color:inherit; }
.study-question { margin:.85em 0; padding:.55em .7em; border-left:.16em solid #498196; }
.study-answer { margin:.45em 0 .9em; padding-left:.7em; border-left:.14em solid #aaa; }
.book02-source { font-size:.9em; }
.book02-source pre { white-space:pre-wrap; tab-size:4; }
.book02-index dt,.book02-glossary dt { font-weight:600; margin-top:.7em; }
.book02-index dd,.book02-glossary dd { margin-left:1em; margin-bottom:.6em; }
.book02-mini {font-size:.92em;}
@media (prefers-color-scheme:dark) {
  .prerequisite-box {background:#1b3039;color:#f0f6f7;}
  .book02-theorem {background:#2b263b;color:#f2eef8;}
}
'''
        self.set(css_path,css.encode('utf8'))
    def fix_crossbook_navigation(self):
        # Historic chapter intro links can point at Book One even when the
        # previous/next label refers to a different book. Replace the actual
        # hrefs, preserving all visible teaching text, with in-book paths.
        updated=0
        for number,path in self.chapter_parts():
            if path!=self.chapter_paths[number][0]:continue
            root=self.get_root(path)
            for a in root.xpath('.//x:a[@href]',namespaces=NS):
                label=norm(''.join(a.itertext())).lower()
                if not ('previous chapter' in label or 'next chapter' in label):continue
                if 'previous' in label:
                    new=(self.chapter_paths[number-1][0]+'#'+self.chapter_roots[number-1]
                         if number>1 else 'EPUB/text/ch005.xhtml')
                else:
                    new=(self.chapter_paths[number+1][0]+'#'+self.chapter_roots[number+1]
                         if number<16 else 'EPUB/text/ch022.xhtml')
                rel=posixpath.relpath(new.split('#')[0],posixpath.dirname(path))
                fragment=('#'+new.split('#',1)[1]) if '#' in new else ''
                if a.get('href')!=rel+fragment:a.set('href',rel+fragment);updated+=1
            if updated:self.set(path,root)
        self.report['fixed_chapter_navigation_links']=updated
    def fix_reader_map_promises(self):
        name='EPUB/text/ch005.xhtml'
        root=self.get_root(name)
        changes=0
        for p in root.xpath('.//x:p',namespaces=NS):
            txt=norm(''.join(p.itertext()))
            if 'Source links resolve to the preserved attachments at the end of this volume' in txt:
                p.text='This volume includes three original archival C++ source listings within Cross Stage References, plus a C++23 Source Companion and tested excerpts. Complete versioned project sources are available at the linked repository.'
                for ch in list(p):p.remove(ch)
                changes+=1
        ref='EPUB/text/ch022.xhtml'
        notes=self.get_root(ref)
        for p in notes.xpath('.//x:p',namespaces=NS):
            if 'original reference notes and source-code attachments are preserved separately' in norm(''.join(p.itertext())).lower():
                p.text='The three original archival C++ source listings appear in Cross Stage References, and the current C++23 project sources are linked in the Source Companion. The following primary references provide further reading.'
                for ch in list(p):p.remove(ch)
                changes+=1
        # A mathematics-first entrance resolves the original Book Two opening:
        # Python stays fully preserved as Chapter 4 rather than dominating the
        # first pages before vectors, frames and linear systems.
        intro=root.xpath('.//x:section[@id="volume-reading-map"]',namespaces=NS)
        if intro:
            p=el('p','Reading route: begin with vectors, frames, and matrix factorizations (Chapters 1–3); Chapter 4 provides the Python tooling and independent numerical visualization companion. Main algorithms and projects remain C++23-first.')
            intro[0].insert(2,p)
        self.set(name,root);self.set(ref,notes)
        self.report['corrected_source_attachment_promises']=changes
    def add_publication_pages(self):
        """Add durable semantic front and back matter using documented sources."""
        # Original one-line copyright left intact in substance, but add the
        # book title, copyright owner and plainly stated non-licensed material.
        name='EPUB/text/ch001.xhtml'; root=self.get_root(name)
        for existing in inside(root,'p'):
            if 'Copyright © 2026 Swayam Singal' in norm(''.join(existing.itertext())):
                container=existing.getparent();idx=container.index(existing)
                container.insert(idx,el('h2','From Equations to Geometry'))
                container.insert(idx+1,el('p','Copyright © 2026 Swayam Singal. All rights reserved.'))
                container.remove(existing)
                container.insert(idx+2,ptext('Digital reader edition · November 2026. Book Two of the seven-book From Bits to Research series.'))
                container.insert(idx+3,ptext('The content is educational and is supplied without a guarantee of suitability for safety-critical, clinical, financial, or other consequential systems. Validate numerical algorithms, conventions, and inputs independently before use.'))
                container.insert(idx+4,ptext('Source-code licensing: the author’s repository and individual source files govern any code permissions. This ebook does not create a separate open-source license. Product and library names may be trademarks of their respective owners.'))
                link=el('p','Companion repository: ')
                a=el('a',SOURCE_REPO,href=SOURCE_REPO);link.append(a)
                container.insert(idx+5,link)
                break
        self.set(name,root)

        # Give an honest explanation of the reference index, rather than
        # treating a cross-book graph as part of the main narrative.
        name='EPUB/text/ch023.xhtml';root=self.get_root(name)
        nodes=root.xpath('.//x:section[@id="cross-stage-reference-index"]',namespaces=NS)
        if nodes:
            explanation=el('p',
               'This archival cross-stage locator records related concepts elsewhere in the seven-book curriculum. These are not chapters missing from Book Two, and cross-volume citations may require the corresponding volume or the project repository. The subject index and glossary in this book are standalone navigational aids.',
               **{'class':'book02-mini'})
            nodes[0].insert(1,explanation)
            self.set(name,root)

        # A real subject index linked to chapter sections, not just a word cloud.
        pages=[]; glossary=[];symbols=[];list_fig=[];source=[];bridge=[]
        figures=section('List of Figures','book02-list-of-figures')
        figures.append(ptext('Every entry links to a numbered illustration in its chapter. Captions describe the actual scientific graphic; the figure itself retains its independent alternative text.'))
        ol=el('ol')
        for figure in self.figure_records:
            li=el('li')
            a=el('a',f"Figure {figure['chapter']}.{figure['number']} — {figure['alt'][:145]}",
                 href=posixpath.basename(figure['file'])+'#'+figure['id'])
            li.append(a);ol.append(li)
        figures.append(ol)
        list_fig.append(figures)
        self.add_back_page('list_figures','List of Figures',list_fig)

        glossary_section=section('Glossary','book02-glossary')
        glossary_section.append(ptext('Brief definitions for navigation; the precise assumptions and signs are established in the relevant chapters.'))
        dl=el('dl',**{'class':'book02-glossary'})
        for term,meaning in sorted(GLOSSARY.items(),key=lambda x:x[0].casefold()):
            dl.append(el('dt',term));dl.append(el('dd',meaning))
        glossary_section.append(dl)
        self.add_back_page('glossary','Glossary', [glossary_section])

        symbols=section('Symbols and Conventions','book02-symbols')
        symbols.append(ptext('The same symbol can have different meanings in separate chapters. These are signposts, not a replacement for each local definition.'))
        table=el('table');thead=el('thead');tr=el('tr')
        for h in ('Symbol','Meaning or convention'):tr.append(el('th',h))
        thead.append(tr);table.append(thead);tb=el('tbody')
        for sign,meaning in SYMBOLS:
            tr=el('tr');tr.append(el('td',sign));tr.append(el('td',meaning));tb.append(tr)
        table.append(tb);symbols.append(table)
        self.add_back_page('symbols','Symbols and Conventions',[symbols])

        source=section('C++23 Source Companion','book02-source-companion')
        source.append(ptext('Core mathematical implementations belong in ISO C++23. Python appears only as an optional plotting or independent reference-checking tool. The full project sources are versioned on GitHub; this appendix identifies them, supplies invocation instructions and records the original source attachment locations without pretending they are already embedded executables.'))
        source.append(el('h2','Build the native workbench'))
        pre=el('pre',
           'cmake -S tools/labs/book02_cpp23 -B build/book02-cpp23 -DCMAKE_BUILD_TYPE=Release\n'
           'cmake --build build/book02-cpp23 --parallel\n'
           'ctest --test-dir build/book02-cpp23 --output-on-failure\n'
           'build/book02-cpp23/book02_workbench orientation-exact\n'
           'build/book02-cpp23/book02_workbench ransac-data > build/book02-cpp23/ransac.csv\n'
           'build/book02-cpp23/book02_workbench heat > build/book02-cpp23/heat.csv\n'
           'build/book02-cpp23/book02_workbench robust-data > build/book02-cpp23/robust.csv')
        source.append(pre)
        link=el('p','Native project: ');link.append(el('a','tools/labs/book02_cpp23',href=SOURCE_REPO+'/tree/'+BRANCH+'/tools/labs/book02_cpp23'));source.append(link)
        source.append(el('h2','Source maps'))
        source_map=[('C++23 numerical workbench entry point','tools/labs/book02_cpp23/src/main.cpp'),
                    ('C++23 exact binary64 predicates','tools/labs/book02_cpp23/src/exact_predicates.hpp'),
                    ('C++23 robust geometric consensus','tools/labs/book02_cpp23/src/geometry.hpp'),
                    ('C++23 pivoted QR and robust fitting','tools/labs/book02_cpp23/src/advanced.hpp'),
                    ('CMake/CTest source','tools/labs/book02_cpp23/CMakeLists.txt')]
        ul=el('ul')
        for title,relative in source_map:
            item=el('li');item.append(el('a',title,href=SOURCE_REPO+'/blob/'+BRANCH+'/'+relative));ul.append(item)
        source.append(ul)
        source.append(el('h2','Three archival source attachments from the print source inventory'))
        source.append(ptext('The three original archival C++ code files are also embedded as readable listings near the end of Cross Stage References. These external repository links identify the exact original source files. Some archival code was authored as C++20; for a C++23-native project, build the current workbench above.'))
        archive=['50214a688c0ef812f3094877b31b37d4d0887b8f17af4479e7a5596ab9977035.cpp',
                 'a41016f2e326b40cdff18ff422f613367046ed9ee69e4f674fd704e2744a14dd.cpp',
                 'c54e3c362767ccd1911378aed10b4550502b7db2f283e39ef3bb62517d609eea.cpp']
        ul=el('ul')
        for i,filename in enumerate(archive,1):
            a=el('a',f'Original attachment {i}: {filename}',
                 href=SOURCE_REPO+'/blob/'+BRANCH+'/Book%20Production/AllMight%20Codex/LaTeX/Generations/.generation-n3azv4es/Assets/Source/'+filename)
            li=el('li');li.append(a);ul.append(li)
        source.append(ul)
        source.append(el('h2','C++23 laboratory with a reproducible oracle'))
        source.append(ptext('Implement a small input contract first, validate it against exact hand cases, then scale it. Here is a standalone 2D vector-and-reflection exercise, not a substitute for the complete library.'))
        code='''// C++23: a reflection with an explicit unit-normal contract.
#include <cmath>
#include <iostream>
#include <stdexcept>
struct Vec2 { double x{}, y{}; };
constexpr double dot(Vec2 a, Vec2 b) { return a.x*b.x + a.y*b.y; }
Vec2 reflect(Vec2 v, Vec2 n) {
    const double nn = dot(n,n);
    if (!std::isfinite(nn) || std::abs(nn-1.0) > 1e-12)
        throw std::invalid_argument("surface normal is not unit length");
    if (!std::isfinite(v.x) || !std::isfinite(v.y))
        throw std::invalid_argument("nonfinite direction");
    const double k = 2.0*dot(v,n);
    return {v.x-k*n.x, v.y-k*n.y};
}
int main() {
    const auto r = reflect({3.0,-4.0},{0.0,1.0});
    if (r.x!=3.0 || r.y!=4.0) return 1;
    std::cout << "reflected=" << r.x << ',' << r.y << '\\n';
}'''
        source.append(el('pre',code,**{'class':'sourceCode cpp'}))
        source.append(ptext('Compile with c++ -std=c++23 -O2 -Wall -Wextra -pedantic file.cpp -o reflect && ./reflect. Expected: reflected=3,4. Extend tests with a zero normal and a NaN input.'))
        source.append(el('h2','C++23 laboratory: exact bounded-integer orientation'))
        source.append(ptext('This small exact predicate is valid only when every input coordinate lies between −10⁶ and +10⁶, so determinant intermediate values fit int64. Arbitrary binary64 exactness is a separate, more advanced problem.'))
        source.append(el('pre', '''// C++23 — exact signed area on a bounded coordinate grid.
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
using Point = std::array<std::int64_t,2>;
int orientation(Point a, Point b, Point c) {
    for (auto p : {a,b,c}) for (auto x : p)
        if (x < -1000000 || x > 1000000)
            throw std::out_of_range("coordinate out of contract");
    const auto ax=a[0]-c[0], ay=a[1]-c[1];
    const auto bx=b[0]-c[0], by=b[1]-c[1];
    const std::int64_t det=ax*by-ay*bx;
    return (det>0)-(det<0);
}
int main() {
    if (orientation({0,0},{2,0},{0,2}) != +1) return 1;
    if (orientation({0,0},{0,2},{2,0}) != -1) return 2;
    if (orientation({0,0},{1,1},{2,2}) != 0) return 3;
    std::cout << "bounded orientation: PASS\\n";
}''',**{'class':'sourceCode cpp'}))
        source.append(ptext('Expected: bounded orientation: PASS. Then replace this bounded oracle with the full finite-binary64 dyadic predicate in the source repository and independently verify its signs.'))
        source.append(el('h2','C++23 laboratory: Monte Carlo estimation and variance'))
        source.append(el('pre', '''// C++23 — estimate integral_0^1 x^2 dx = 1/3.
#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <stdexcept>
int main() {
    std::mt19937_64 engine{73013};
    std::uniform_real_distribution<double> uniform{0.0,1.0};
    constexpr std::size_t samples=100000;
    double mean=0.0, m2=0.0;
    for (std::size_t i=1;i<=samples;++i) {
        const double x=uniform(engine), y=x*x;
        const double delta=y-mean;
        mean+=delta/static_cast<double>(i);
        m2+=delta*(y-mean);
    }
    const double sampleVariance=m2/(samples-1);
    const double stdError=std::sqrt(sampleVariance/samples);
    if (!std::isfinite(stdError) ||
        std::abs(mean-1.0/3.0)>0.02)
        throw std::runtime_error("integral oracle mismatch");
    std::cout << "estimate=" << mean
              << ", std_error=" << stdError << '\\n';
}''',**{'class':'sourceCode cpp'}))
        source.append(ptext('The output is stochastic. The exact integral 1/3 gives an independent reference; replicate the entire run over independent seeds before interpreting uncertainty.'))
        source.append(el('h2','C++23 laboratory: a pivoted two-equation solve'))
        source.append(el('pre', '''// C++23 — teaching-sized pivoted Gaussian elimination.
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <optional>
using Row=std::array<double,3>;
std::optional<std::array<double,2>> solve(Row a,Row b) {
    for (double x:a) if (!std::isfinite(x)) return std::nullopt;
    for (double x:b) if (!std::isfinite(x)) return std::nullopt;
    const double s=std::max({std::abs(a[0]),std::abs(a[1]),
                             std::abs(b[0]),std::abs(b[1])});
    if (!(s>0)) return std::nullopt;
    for (double& x:a) x/=s;
    for (double& x:b) x/=s;
    if (std::abs(b[0])>std::abs(a[0])) std::swap(a,b);
    const double eps=64*std::numeric_limits<double>::epsilon();
    if (std::abs(a[0])<=eps) return std::nullopt;
    const double ratio=b[0]/a[0];
    b[1]-=ratio*a[1]; b[2]-=ratio*a[2];
    if (std::abs(b[1])<=eps) return std::nullopt;
    const double y=b[2]/b[1], x=(a[2]-a[1]*y)/a[0];
    if (!std::isfinite(x)||!std::isfinite(y)) return std::nullopt;
    return std::array<double,2>{x,y};
}
int main() {
    const auto result=solve({2,1,5},{1,1,3});
    if (!result || std::abs((*result)[0]-2)>1e-12 ||
                   std::abs((*result)[1]-1)>1e-12) return 1;
    std::cout << "pivoted solve: PASS\\n";
}''',**{'class':'sourceCode cpp'}))
        source.append(ptext('Expected: pivoted solve: PASS. Test singular and badly conditioned inputs. The pivot threshold and floating policy do not constitute a general rank-revealing solver.'))
        self.add_back_page('source_companion','C++23 Source Companion',[source])

        b=section('Further Reading and Source Ledger','book02-further-reading')
        b.append(ptext('Primary papers and authoritative implementation references for research reconstruction. Links are reading directions, not claims that this book reproduces the publications’ full benchmarks. The attached manuscript’s original reading list is retained separately.'))
        papers=[('Robust orientation predicates','Jonathan R. Shewchuk (1997), Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates','https://www.cs.cmu.edu/~quake/robust.html'),
                ('Geometric distance fields','Keenan Crane, Clarisse Weischedel, Max Wardetzky (2013), Geodesics in Heat','https://www.cs.cmu.edu/~kmcrane/Projects/HeatMethod/'),
                ('Random consensus','Martin A. Fischler and Robert C. Bolles (1981), Random Sample Consensus','https://doi.org/10.1145/358669.358692'),
                ('Monte Carlo sampling','Eric Veach (1997), Robust Monte Carlo Methods for Light Transport Simulation','https://graphics.stanford.edu/papers/veach_thesis/'),
                ('Surface discretization','libigl tutorial, discrete differential-geometry operators','https://libigl.github.io/tutorial/'),
                ('Nonlinear least squares','Ceres Solver documentation, nonlinear least squares and robust loss','https://ceres-solver.readthedocs.io/latest/nnls_modeling.html'),
                ('Scientific numerics','SciPy reference manual, optimization and numerical integration','https://docs.scipy.org/doc/scipy/reference/'),
                ('C++23 language','ISO C++ community working draft and references','https://isocpp.org/'),
               ]
        ul=el('ul')
        for topic,name,url in papers:
            li=el('li',topic+' — ');li.append(el('a',name,href=url));ul.append(li)
        b.append(ul)
        self.add_back_page('further_reading','Further Reading and Source Ledger',[b])

        bridge=section('From Book Two to Book Three','book02-bridge')
        bridge.append(ptext('You can now take mathematical objects, numerical contracts, coordinate transformations, and measured uncertainty into complete images. Book Three develops geometry, cameras, and image formation. To prepare, revisit the projects on transforms, projection Jacobians, robust predicates, normals, and reconstruction error; carry your tested C++23 workbench forward.'))
        bridge.append(ptext('A recommended transition project: load a small mesh, transform it with an explicitly documented frame convention, project vertices to a virtual camera, classify triangle coverage with a robust orientation predicate, and export an image plus numerical error report. Report degeneracy and invalid inputs; never turn an indeterminate numerical result into a claimed geometric fact.'))
        self.add_back_page('book_three_bridge','What Comes Next',[bridge])

    def add_back_page(self,slug,title,nodes):
        path='EPUB/text/book02-'+slug+'.xhtml'
        self.set(path,fresh_page(title,nodes))
        self.add_item('book02_'+slug,'text/book02-'+slug+'.xhtml',
                      'application/xhtml+xml',spine=True)
        return path
# Four precise, answerable tasks per chapter, followed by four C++23
# implementation and scientific-verification tasks. None are filler recall.
PRACTICE={
1:[
 ('Evaluate the machine-facing expression (10^16 + 1) − 10^16 in binary64 when each operation rounds to binary64.','The increment 1 is below the spacing at 10^16; the first addition rounds to 10^16 and the result is 0, unlike exact arithmetic.'),
 ('For data [1, 2, 3], compute the population mean and population variance.','Mean 2; variance ((1−2)^2 + 0 + (3−2)^2)/3 = 2/3.'),
 ('Explain why vectorized NumPy operations may be faster but do not guarantee zero copies.','They shift loops to compiled kernels; dtype conversion, layout, broadcasting, or intermediate arrays can still allocate or copy.'),
 ('How should a scientific CSV export specify a seed and units?','Record the actual seed, generator and version, column meanings and units, input sizes, algorithm version, and test oracle; a seed alone is insufficient.')],
2:[
 ('Reflect v=(3,−4) across a surface with unit normal n=(0,1).','r=v−2(v·n)n=(3,4).'),
 ('Find the barycentric weights of P=(0.5,0.5) in triangle A=(0,0), B=(2,0), C=(0,2).','P=(1/2)A+(1/4)B+(1/4)C; weights (1/2, 1/4, 1/4).'),
 ('Derive the reflection formula by decomposing a vector into tangent and normal components.','v_t=v−(v·n)n and v_n=(v·n)n for unit n; reflection v_t−v_n=v−2(v·n)n.'),
 ('Show why n=(0,2) must not be passed into the unit-normal reflection formula without normalization.','It produces v−2(v·n)n, multiplying the projected normal by n·n=4. Correct general formula divides by n·n.')],
3:[
 ('Represent P=(3,4) in the basis b₁=(1,0), b₂=(0,2).','P=3b₁+2b₂; coordinate pair (3,2).'),
 ('With rotation R through +90° and translation t=(1,2), find T(P) for P=(2,0).','R P=(0,2); add t to obtain (1,4).'),
 ('Explain why a direction transforms differently from a point under a homogeneous affine matrix.','Points have homogeneous weight 1 and receive translation; directions have weight 0 and do not.'),
 ('Explain why rotation then translation generally differs from translation then rotation.','Matrix multiplication is noncommutative; R(P+t)=RP+Rt differs from RP+t unless t is invariant under R.')],
4:[
 ('Solve 2x+y=5 and x+y=3 by elimination.','Subtract the second equation from the first: x=2; substitute to find y=1.'),
 ('Compute the Cholesky factor L for [[4,2],[2,5]], using A=LLᵀ.','L=[[2,0],[1,2]]; the reconstructed product is [[4,2],[2,5]].'),
 ('State when a real symmetric matrix has a unique Cholesky factor with positive diagonal.','When it is positive definite; symmetry alone or semidefiniteness does not guarantee the required positive pivots.'),
 ('Why can solving normal equations worsen conditioning compared with QR?','Under full column rank, κ₂(AᵀA)=κ₂(A)²; forming the product magnifies ill-conditioning.')],
5:[
 ('Compute ∇f(3,4) for f(x,y)=x²+y².','∇f=(2x,2y)=(6,8).'),
 ('For g(x,y)=(xy,x+y), compute its Jacobian at (2,3).','J=[[y,x],[1,1]]=[[3,2],[1,1]].'),
 ('Derive the chain rule for y=f(g(x)) in matrix-Jacobian form.','D(f∘g)(x)=Df(g(x))·Dg(x), assuming the stated differentiability and compatible dimensions.'),
 ('Give an example of a derivative that exists analytically but is numerically unstable under naive finite differences.','Subtract nearly equal values f(x+h)−f(x) with tiny h; floating-point cancellation overwhelms truncation improvement.')],
6:[
 ('Integrate x² over [0,1].','The antiderivative is x³/3; exact integral 1/3.'),
 ('Estimate ∫₀¹ x² dx using the one-panel trapezoid rule.','(f(0)+f(1))/2 = 1/2; signed estimate error +1/6.'),
 ('Explain why Simpson’s rule integrates a cubic polynomial exactly over one panel.','Its interpolating quadratic quadrature has degree of precision 3 under the standard even-subinterval convention.'),
 ('What happens if a numerical quadrature rule is refined without controlling function singularities?','The nominal smoothness/error assumptions fail; convergence may be slow, irregular, or absent.')],
7:[
 ('Find the probability that two fair six-sided dice sum to seven.','Six favorable ordered outcomes out of 36, so 1/6.'),
 ('Compute the variance of a Bernoulli random variable with p=0.2.','p(1−p)=0.2·0.8=0.16.'),
 ('Explain why a probability density may exceed 1 while total probability remains 1.','Density is probability per unit measure; probabilities are integrals over sets, not pointwise density values.'),
 ('Distinguish independent random variables from variables with zero covariance.','Independence implies zero covariance when moments exist, but zero covariance does not imply independence in general.')],
8:[
 ('For population data [1,2,3], compute variance.','The mean is 2; population variance 2/3.'),
 ('If X=2Y+3 and Var(Y)=5, compute Var(X).','Var(X)=2²·5=20; adding a constant does not change variance.'),
 ('Derive Cov(aX+b,cY+d) for finite second moments.','It is ac·Cov(X,Y), because centering removes constants.'),
 ('Explain why an almost-zero singular value can destabilize an inverse.','The inverse scales that singular direction by 1/σ; small measurement errors can be strongly amplified.')],
9:[
 ('Take one forward-Euler step for y′=−y from y₀=1 with Δt=0.1.','y₁=y₀−Δt·y₀=0.9.'),
 ('For a one-dimensional explicit heat stencil, what upper bound on r=αΔt/Δx² gives the usual nonnegative three-point weights?','0≤r≤1/2 for the standard one-dimensional explicit central-difference stencil.'),
 ('Derive the Euler local update from y(t+Δt)≈y(t)+Δt·y′(t).','Evaluate the derivative at the current state: y_{n+1}=y_n+Δt f(t_n,y_n).'),
 ('Why does a smaller time step not automatically make an unstable physical model correct?','It may reduce discretization error but cannot repair wrong forces, boundary conditions, units, or modeling assumptions.')],
10:[
 ('Minimize f(θ)=(θ−2)² over real θ.','Derivative 2(θ−2)=0 gives θ=2, global minimum since f≥0.'),
 ('For model y=a exp(bx)+c, compute ∂y/∂a and ∂y/∂b.','The derivatives are exp(bx) and a x exp(bx), respectively.'),
 ('Derive the first-order normal equation for minimizing ‖JΔ+r‖².','Set gradient 2Jᵀ(JΔ+r)=0, obtaining JᵀJΔ=−Jᵀr.'),
 ('Why is a small squared residual not proof of recovering true parameters?','Different parameters can fit noisy or nonidentifiable data; regularization and identifiability matter.')],
11:[
 ('Minimize (θ−1)²+(θ−3)².','Derivative 2(θ−1)+2(θ−3)=4θ−8 gives θ=2.'),
 ('For residual r(θ)=[θ−1,θ−3]ᵀ at θ=0, compute one Gauss–Newton step.','J=[1,1]ᵀ, JᵀJ=2 and −Jᵀr=4, so Δ=2.'),
 ('Explain how gauge freedom appears as rank deficiency in a pose-fitting Jacobian.','A transformation that leaves the observations unchanged produces a Jacobian null direction; parameters are not uniquely identified.'),
 ('Why must a computed Gauss–Newton step be checked before acceptance?','Linearization is local; evaluate objective reduction and domain constraints, especially for large steps or poor starting points.')],
12:[
 ('Apply one Newton update to x²−2=0 from x₀=1.5.','x₁=(x₀+2/x₀)/2≈1.4166666667.'),
 ('Find the first bisection midpoint for x²−2 on [1,2] and select the next bracket.','Midpoint 1.5 has positive residual, so retain [1,1.5].'),
 ('Describe the stationary point test for an unconstrained differentiable objective.','∇f=0 is necessary at an interior smooth local extremum; it is not sufficient for a minimum.'),
 ('Explain how Newton steps fail when the derivative is near zero.','The update −f/f′ can be enormous or undefined; guard derivatives and use bracketing/globalization.')],
13:[
 ('Estimate f′(1) for f(x)=x² with forward step h=0.1.','((1.1)²−1)/0.1=2.1; exact derivative is 2, error +0.1.'),
 ('If an error behaves as Ch, what happens when h is halved?','The leading error approximately halves, assuming the asymptotic regime and fixed nonzero C.'),
 ('State the distinction between consistency, stability, and convergence.','Consistency concerns approximation of the intended equation, stability controls perturbation growth, convergence concerns approach to the intended solution.'),
 ('Give an adversarial case where a measured convergence order is misleading.','Changing physical domain, solver tolerance, mesh quality, or stochastic seed with resolution can contaminate the error ratio.')],
14:[
 ('For uniform X on [0,1], compute E[X] and Var(X).','E[X]=1/2; Var(X)=1/12.'),
 ('For N=100 independent uniform-[0,1] samples, compute the standard deviation of the sample mean.','sqrt((1/12)/100)=1/sqrt(1200)≈0.0288675.'),
 ('Derive the variance of the independent sample mean.','Var((1/N)∑Xᵢ)=Nσ²/N²=σ²/N, assuming independent equal-variance samples.'),
 ('Why does an incorrect proposal distribution invalidate a Monte Carlo importance estimate?','The weighting and support no longer correspond to the claimed target integral, causing bias or undefined weights.')],
15:[
 ('Normalize nonnegative weights [1,2,1] into selection probabilities.','Divide by total 4: [0.25,0.5,0.25].'),
 ('Compute sample mean and unbiased sample variance for [1,2,3].','Mean 2; sum of squared deviations 2, divide by N−1=2 to obtain variance 1.'),
 ('Explain why every C++23 probability sampler needs an explicit support contract.','A zero/negative/NaN weight or missing support can lead to invalid distributions and undefined estimators.'),
 ('Why do identical pseudorandom seeds not guarantee identical floating samples on every C++ standard library?','Generator engines can be standardized while distribution mapping algorithms and floating behavior may differ by implementation.')],
16:[
 ('For independent observations with variance 4 and N=100, calculate the mean’s standard error.','sqrt(4/100)=0.2.'),
 ('A test with 90% sensitivity, 95% specificity, and 1% prevalence is positive. Estimate the positive predictive value.','0.9·0.01/(0.9·0.01+0.05·0.99)=0.009/0.0585≈0.15385.'),
 ('Distinguish a 95% confidence interval from a 95% posterior credible interval.','The confidence procedure concerns long-run coverage; a credible interval uses a stated prior/model to assign posterior probability.'),
 ('Why should multiple samples from the same run not automatically be treated as independent trials?','Correlation or shared hidden factors reduce effective independent information and can understate uncertainty.')],
}

LABS={
1:('streaming-mean','mean, variance, finite-input rejection','one-pass Welford summary','empty input, extreme scale'),
2:('vector-frame','dot, projection, reflection, barycentric weights','hand-derived right triangle and reflected ray','zero normal, collinear triangle'),
3:('affine-frames','point/direction transform and inverse checks','explicit coordinate-basis round trip','singular basis, incompatible frame units'),
4:('pivoted-solve','2×2 elimination and Householder QR comparison','independently derived small-system solution','rank deficiency, near-dependent columns'),
5:('jacobian-check','analytical Jacobian and central finite differences','symbolic derivative plus step-size sweep','small step cancellation and discontinuity'),
6:('quadrature','trapezoid and Simpson rules','polynomial exact integral oracle','endpoint discontinuity and singular integrand'),
7:('discrete-rv','normalized PMF and weighted expectation','hand-enumerated finite sample space','negative weights and nonnormalized probabilities'),
8:('covariance-matrix','streaming covariance and 2×2 eigen decomposition','explicit centered observation matrix','nearly singular covariance and large offsets'),
9:('heat-step','1D explicit heat diffusion with fixed boundaries','one-pulse first-step hand calculation','r>1/2 and nonfinite diffusivity'),
10:('nonlinear-model','parameter Jacobian and damped robust-fit step','finite-difference cross-check and synthetic truth','nonidentifiability and outliers'),
11:('pose-fit','Gauss–Newton residual and QR step','synthetic small pose with known transformation','unfixed gauge and singular Jacobian'),
12:('root-solver','bracketed bisection and guarded Newton step','known polynomial root and residual','zero derivative and unbracketed root'),
13:('convergence-sweep','fixed-domain finite-difference refinement','analytical derivative/error order','changing domain and roundoff floor'),
14:('monte-carlo','uniform Monte Carlo with Welford variance','known integral and multi-seed confidence check','correlated samples, wrong weights'),
15:('sampling-contract','discrete weighted sampling with validation','enumerated PMF and empirical frequencies','invalid CDF and zero-support proposal'),
16:('inference-study','uncertainty interval over repeated independent runs','exact synthetic model and independent replicates','pseudoreplication and selection bias'),
}


def chapter_extra_exercises(n):
    cmd,scope,oracle,adversarial=LABS[n]
    return [
      (f'Implement the {cmd} C++23 laboratory as a named function with documented input and output types; cover {scope}.',
       f'A solution must compile as C++23, expose the input domain, report invalid arguments explicitly, and compare against {oracle}.'),
      (f'Create a deterministic CTest fixture for the {cmd} laboratory and compare its output with an independent oracle.',
       f'Use a separate hand or reference calculation ({oracle}); record the exact inputs and tolerances, not only a passing assertion.'),
      (f'Break a naive {cmd} implementation using {adversarial}. Explain which assumption was violated.',
       f'The adversarial input must be rejected or handled by a documented policy; do not label uncertain or nonfinite output as success.'),
      (f'Export CSV/JSON evidence for {cmd}. How would you distinguish correctness, conditioning, and runtime?',
       'Provide names and units, revision, seed if randomized, numerical oracle and errors, compiler settings and run count. Correctness is not a measured performance improvement.'),
    ]
class Editor(Editor):
    def add_workbook(self):
        wb=section('Practice Workbook · 128 Exercises','book02-practice-workbook')
        wb.append(ptext('Each chapter has eight graded questions: four mathematical exercises and four C++23 implementation, testing, failure-analysis and reporting tasks. The independent hints/answers appear later in the book. These are new, additive learning aids, not replacements for the original manuscript.'))
        ol=el('ol')
        for n in range(1,17):
            li=el('li');li.append(el('a',f'Chapter {n}: {self.chapter_titles[n]}',href=f'book02-exercises-{n:02}.xhtml#book02-exercises-{n}'));ol.append(li)
        wb.append(ol)
        self.add_back_page('workbook','Practice Workbook · 128 Exercises',[wb])
        for n in range(1,17):
            sec=section(f'Exercises · Chapter {n}: {self.chapter_titles[n]}',f'book02-exercises-{n}')
            sec.append(ptext('Complete the hand calculations before programming. Record units, assumptions and numerical policy. Compile C++23 implementations with warnings enabled and compare against an independent oracle.'))
            original_n=self.reading_order[n-1]
            questions=PRACTICE[original_n]+chapter_extra_exercises(original_n)
            assert len(questions)==8
            for i,(question,answer) in enumerate(questions,1):
                p=el('p',**{'class':'study-question','id':f'book02-exercise-{n}-{i}'})
                p.append(el('strong',f'{n}.{i} · '))
                s=el('span',question);p.append(s);sec.append(p)
            jump=el('p','Hints and answer checks: ')
            jump.append(el('a',f'Chapter {n} answer key',href=f'book02-answers-{n:02}.xhtml#book02-answers-{n}'))
            sec.append(jump)
            self.add_back_page(f'exercises-{n:02}',f'Exercises — Chapter {n}',[sec])
        ans=section('Hints, Solutions, and Checking Criteria','book02-answer-key')
        ans.append(ptext('Numbers and derivations are answered directly. Open-ended engineering work is assessed by explicit validation conditions rather than misleadingly pretending a single implementation is uniquely correct.'))
        ol=el('ol')
        for n in range(1,17):
            li=el('li');li.append(el('a',f'Chapter {n} answers',href=f'book02-answers-{n:02}.xhtml#book02-answers-{n}'));ol.append(li)
        ans.append(ol)
        self.add_back_page('answer_key','Hints and Answer Key',[ans])
        for n in range(1,17):
            sec=section(f'Answer Key · Chapter {n}',f'book02-answers-{n}')
            sec.append(ptext('Return to the corresponding question and reconstruct the reasoning independently. Include your own tests for input shapes, units, tolerances, and exceptions.'))
            original_n=self.reading_order[n-1]
            questions=PRACTICE[original_n]+chapter_extra_exercises(original_n)
            for i,(_q,answer) in enumerate(questions,1):
                p=el('p',**{'class':'study-answer'})
                p.append(el('strong',f'{n}.{i} · '));p.append(el('span',answer));sec.append(p)
            self.add_back_page(f'answers-{n:02}',f'Answers — Chapter {n}',[sec])
        self.report['new_practice_exercises']=16*8
        self.report['answer_or_checking_entries']=16*8

    def add_index(self):
        # Exact section references, based on headings in the complete original
        # chapter text. If a term exists only inside prose, the locator points
        # to its chapter start rather than inventing an ungrounded subheading.
        targets={}
        for n in range(1,17):
            section_positions=[];whole=[]
            for path in self.chapter_paths[n]:
                root=self.get_root(path)
                bb=body(root)
                text=norm(' '.join(bb.itertext()))
                whole.append(text)
                root_anchor=self.chapter_roots[n]
                spans=[x for x in bb.iter(Q(X,'span')) if x.get('class')=='section-anchor' and x.get('id')]
                for span in spans:
                    heading=next((x for x in span.itersiblings() if local(x) in ('h2','h3')),None)
                    if heading is not None:
                        section_positions.append((norm(''.join(heading.itertext())),path,span.get('id')))
            full=' '.join(whole)
            for term in TERMS:
                if re.search(r'(?<!\w)'+re.escape(term)+r'(?!\w)',full,re.IGNORECASE) is None:
                    continue
                heading_hits=[(name,path,ident) for name,path,ident in section_positions
                              if re.search(r'(?<!\w)'+re.escape(term)+r'(?!\w)',name,re.IGNORECASE)]
                links=heading_hits[:2] if heading_hits else [(self.chapter_titles[n],self.chapter_paths[n][0],self.chapter_roots[n])]
                for name,path,ident in links:
                    targets.setdefault(term,[]).append((n,name,path,ident))
        sec=section('Subject Index','book02-subject-index')
        sec.append(ptext('An index of topics actually present in Book Two. Locators lead to chapter sections whenever a matching heading exists; otherwise they lead to the chapter opening. This is distinct from the cross-volume reference index.'))
        dl=el('dl',**{'class':'book02-index'})
        for term,locs in sorted(targets.items(),key=lambda x:x[0].casefold()):
            dl.append(el('dt',term[0].upper()+term[1:]))
            dd=el('dd')
            seen=set()
            for n,title,path,ident in locs:
                key=(n,path,ident)
                if key in seen:continue
                seen.add(key)
                if len(seen)>1:
                    sep=el('span','; ');dd.append(sep)
                link=f'{posixpath.basename(path)}#{urllib.parse.quote(ident,safe="-._~")}'
                dd.append(el('a',f'Ch. {n} — {title[:65]}',href=link))
                if len(seen)>=6:break
            dl.append(dd)
        sec.append(dl)
        self.add_back_page('index','Subject Index',[sec])
        self.report['subject_index_terms']=len(targets)
        self.report['subject_index_locators']=sum(min(6,len(v)) for v in targets.values())

    def relink_all(self):
        """Update moved deep links, respecting IDs scoped to each XHTML page."""
        moved={}
        for c in range(1,17):
            source=self.chapter_paths[c][0]
            for name in self.chapter_paths[c]:
                root=parse(self.data[name])
                for node in root.iter():
                    ident=node.get('id')
                    if not ident:continue
                    key=(source,ident)
                    if key in moved:
                        raise RuntimeError('Duplicate ID within split chapter: '+ident)
                    moved[key]=name
        extra=getattr(self,'extra_split_mapping',{})
        for src,parts in extra.items():
            for file in parts:
                doc=parse(self.data[file])
                for node in doc.iter():
                    ident=node.get('id')
                    if ident:
                        key=(src,ident)
                        if key in moved:
                            raise RuntimeError('Duplicate ID within original references: '+ident)
                        moved[key]=file
        self.ids=moved
        chapter_for_part={path:paths[0] for paths in self.chapter_paths.values() for path in paths}
        for src,parts in extra.items():
            for path in parts:chapter_for_part[path]=src
        changes=0
        for name,data in list(self.data.items()):
            if not name.endswith(('.xhtml','.ncx')):continue
            root=parse(data)
            dirty=False
            for node in root.iter():
                for attr in ('href','src'):
                    href=node.get(attr)
                    if not href or href.startswith(('http:','https:','mailto:','data:','tel:','//')):
                        continue
                    parsed=urllib.parse.urlsplit(href)
                    if not parsed.fragment:continue
                    fragment=urllib.parse.unquote(parsed.fragment)
                    referenced=posixpath.normpath(posixpath.join(
                        posixpath.dirname(name),urllib.parse.unquote(parsed.path))) if parsed.path else name
                    original=chapter_for_part.get(referenced,referenced)
                    intended=moved.get((original,fragment))
                    if intended is None or referenced==intended:continue
                    rel=posixpath.relpath(intended,posixpath.dirname(name))
                    node.set(attr,urllib.parse.urlunsplit((parsed.scheme,
                       parsed.netloc,rel,parsed.query,parsed.fragment)))
                    dirty=True;changes+=1
            if dirty:self.set(name,ser(root))
        self.report['relinked_deep_links']=changes

    def clean_namespace_and_nav(self):
        nav_path='EPUB/nav.xhtml';root=parse(self.data[nav_path])
        tocs=[n for n in root.iter() if n.tag==Q(X,'nav') and n.get('id')=='toc']
        if len(tocs)!=1:raise RuntimeError('EPUB toc navigation missing')
        toc=tocs[0];lists=[x for x in toc if x.tag==Q(X,'ol')]
        if len(lists)!=1:raise RuntimeError('Unexpected toc structure')
        parent=lists[0]
        # Keep every nested h2 deep link and move whole TOC chapter branches
        # into the same order as the OPF spine; add explicit 1–16 numbering.
        originals=list(parent)
        chapter_li={}
        for li in originals:
            direct=[x for x in li if x.tag==Q(X,'a')]
            if not direct:continue
            href=direct[0].get('href','')
            for number in range(1,17):
                orig=self.reading_order[number-1]
                if href.startswith(f'text/ch{orig+5:03}.xhtml'):
                    chapter_li[number]=li
                    direct[0].text=f'Chapter {number} · '+norm(''.join(direct[0].itertext()))
                    break
        if len(chapter_li)!=16:raise RuntimeError('Cannot map all 16 TOC chapters')
        first=min(parent.index(node) for node in chapter_li.values())
        for li in chapter_li.values():parent.remove(li)
        for offset,n in enumerate(range(1,17)):
            parent.insert(first+offset,chapter_li[n])
        extra=[('C++23 Source Companion','book02-source_companion'),
               ('Practice Workbook · 128 Exercises','book02-workbook'),
               ('Hints and Answer Key','book02-answer_key'),
               ('List of Figures','book02-list_figures'),
               ('Symbols and Conventions','book02-symbols'),
               ('Glossary','book02-glossary'),
               ('Further Reading and Source Ledger','book02-further_reading'),
               ('Subject Index','book02-index'),
               ('What Comes Next','book02-book_three_bridge')]
        for title,slug in extra:
            li=el('li');li.append(el('a',title,href='text/book02-'+slug.removeprefix('book02-')+'.xhtml'))
            parent.append(li)
        # TOC entries MUST follow the actual OPF reading spine. The earlier
        # hard-coded back-matter order placed the answer key ahead of the
        # figures and glossary, triggering three EPUBCheck NAV-011 warnings.
        # Keep nested chapter deep links unchanged, sorting top-level items.
        spine_rank={self.items[ref.get('idref')].get('href'):position
                    for position,ref in enumerate(self.spine)
                    if ref.get('idref') in self.items}
        def rank_href(href):
            target=urllib.parse.unquote(urllib.parse.urlsplit(href or '').path)
            if target not in spine_rank:
                raise RuntimeError('TOC target missing from reading spine: '+str(href))
            return spine_rank[target]
        def top_link(li):
            anchors=[x for x in li if x.tag==Q(X,'a')]
            if len(anchors)!=1:
                raise RuntimeError('Expected one direct TOC link per top-level entry')
            return anchors[0].get('href')
        ordered=sorted(list(parent),key=lambda li:rank_href(top_link(li)))
        for item in list(parent):parent.remove(item)
        for item in ordered:parent.append(item)
        self.report['toc_spine_order_verified']=True

        # EPUB 3 allows only one landmarks navigation. Pandoc already
        # contributes one, so update that block rather than appending a
        # second landmarks navigation. doc-landmarks is NOT an allowed
        # ARIA navigation role (EPUBCheck RSC-005).
        landmarks=[n for n in root.iter(Q(X,'nav'))
                   if 'landmarks' in n.get(Q(EPUB,'type'),'').split()]
        if not landmarks:
            landmark=E.SubElement(body(root),Q(X,'nav'),id='landmarks')
        else:
            landmark=landmarks[0]
            for duplicate in landmarks[1:]:
                duplicate.getparent().remove(duplicate)
        landmark.set(Q(EPUB,'type'),'landmarks')
        landmark.attrib.pop('role',None)
        for child in list(landmark):landmark.remove(child)
        landmark.append(el('h2','Landmarks'))
        ol=E.SubElement(landmark,Q(X,'ol'))
        for title,url,epub_type in [
            ('Title page','text/title_page.xhtml','titlepage'),
            ('Begin reading','text/ch006.xhtml','bodymatter'),
            ('Practice workbook','text/book02-workbook.xhtml','appendix'),
            ('Subject index','text/book02-index.xhtml','index')]:
            li=E.SubElement(ol,Q(X,'li'))
            a=E.SubElement(li,Q(X,'a'),href=url)
            a.set(Q(EPUB,'type'),epub_type);a.text=title
        # Recreate root with natural default XHTML namespace, no ns0 remnants.
        new=E.Element(Q(X,'html'),nsmap={None:X,'epub':EPUB})
        for k,v in root.attrib.items():new.set(k,v)
        new.text=root.text
        for child in list(root):new.append(child)
        self.set(nav_path,ser(new))

        # The NCX fallback is useful for older readers; update new book parts,
        # and make the default NCX namespace human-readable instead of ns0.
        ncx_path='EPUB/toc.ncx';r=parse(self.data[ncx_path])
        maps=r.xpath('.//ncx:navMap',namespaces=NS)
        if len(maps)!=1:raise RuntimeError('NCX navMap missing')
        nm=maps[0]
        chapter_ncx={}
        for item in list(nm):
            if item.tag!=Q(NCX,'navPoint'):continue
            direct=[x for x in item if x.tag==Q(NCX,'content')]
            if not direct:continue
            href=direct[0].get('src','')
            for n in range(1,17):
                orig=self.reading_order[n-1]
                if href.startswith(f'text/ch{orig+5:03}.xhtml'):
                    chapter_ncx[n]=item
                    label=item.find(Q(NCX,'navLabel'))
                    text=label.find(Q(NCX,'text')) if label is not None else None
                    if text is not None:text.text=f'Chapter {n} · '+(text.text or '')
                    break
        if len(chapter_ncx)!=16:raise RuntimeError('Cannot map all 16 NCX chapters')
        first=min(nm.index(node) for node in chapter_ncx.values())
        for item in chapter_ncx.values():nm.remove(item)
        for offset,n in enumerate(range(1,17)):
            nm.insert(first+offset,chapter_ncx[n])
        count=len(r.xpath('.//ncx:navPoint',namespaces=NS))
        for i,(title,slug) in enumerate(extra,1):
            node=E.SubElement(nm,Q(NCX,'navPoint'),id=f'book02-new-nav-{i}')
            lab=E.SubElement(node,Q(NCX,'navLabel'))
            E.SubElement(lab,Q(NCX,'text')).text=title
            E.SubElement(node,Q(NCX,'content'),src='text/book02-'+slug.removeprefix('book02-')+'.xhtml')
        # Align NCX fallback with the same OPF reading order as nav.xhtml.
        def ncx_rank(point):
            content=point.find(Q(NCX,'content'))
            if content is None:
                raise RuntimeError('NCX top-level navPoint lacks a content link')
            return rank_href(content.get('src'))
        ordered_ncx=sorted(list(nm),key=ncx_rank)
        for point in list(nm):nm.remove(point)
        for point in ordered_ncx:nm.append(point)
        self.report['ncx_spine_order_verified']=True
        # Fix the depth value based on actual nested entries, not a constant.
        def dpt(element,depth=0):
            candidates=[dpt(child,depth+1) for child in element
                        if child.tag==Q(NCX,'navPoint')]
            return max([depth]+candidates)
        depth=max([dpt(c,1) for c in nm if c.tag==Q(NCX,'navPoint')],default=1)
        for meta in r.xpath('.//ncx:meta',namespaces=NS):
            if meta.get('name')=='dtb:depth':meta.set('content',str(depth))
        new=E.Element(Q(NCX,'ncx'),nsmap={None:NCX})
        for k,v in r.attrib.items():new.set(k,v)
        new.text=r.text
        for child in list(r):new.append(child)
        self.set(ncx_path,ser(new))
        self.report['nav_entries']=len(toc.xpath('.//x:li',namespaces=NS))
        self.report['ncx_entries']=count+len(extra)
        self.report['nav_namespace_prefixes_cleaned']=True


    def complete_math_accessibility(self):
        repaired=0
        for name in list(self.data):
            if not name.endswith('.xhtml'):continue
            root=parse(self.data[name]);dirty=False
            for formula in root.iter(Q(M,'math')):
                if formula.get('alttext'):continue
                desc=norm(''.join(formula.itertext()))
                if desc:formula.set('alttext',desc[:260]);dirty=True;repaired+=1
            if dirty:self.set(name,root)
        self.report['remaining_mathml_alttext_added']=repaired

    def canonicalize_generated_anchors(self):
        """Replace machine-looking generated anchor IDs with readable slugs.

        Anchor substitutions are scoped to their real XHTML document, and all
        cross-document deep links are rewritten. Original prose, figures,
        code, and MathML text are byte-semantically unchanged.
        """
        replacements={};changed_docs=0
        for name,data in list(self.data.items()):
            if not name.endswith('.xhtml'):continue
            root=parse(data); seen={n.get('id') for n in root.iter() if n.get('id')}
            used=set(seen);dirty=False
            for node in root.iter():
                ident=node.get('id')
                if not ident or not ident.startswith('id_'):continue
                match=re.search(r'-ch\d+-(.*)',ident)
                candidate=match.group(1) if match else ident[3:]
                candidate=unicodedata.normalize('NFKD',candidate).encode('ascii','ignore').decode('ascii').lower()
                candidate=re.sub('[^a-z0-9]+','-',candidate).strip('-')[:65].rstrip('-')
                file_stem=pathlib.Path(name).stem
                new=f'b2-{file_stem}-{candidate}'
                if not candidate:new=f'b2-{file_stem}-anchor'
                suffix=2
                while new in used:
                    new=f'b2-{file_stem}-{candidate}-{suffix}'
                    suffix+=1
                used.add(new)
                node.set('id',new)
                replacements[(name,ident)]=new
                dirty=True
            if dirty:self.set(name,root);changed_docs+=1
        changed_links=0
        for name,data in list(self.data.items()):
            if not name.endswith(('.xhtml','.ncx')):continue
            root=parse(data);dirty=False
            for node in root.iter():
                for field in ('href','src'):
                    value=node.get(field)
                    if not value or value.startswith(('http:','https:','mailto:','data:','tel:','//')):continue
                    pieces=urllib.parse.urlsplit(value)
                    if not pieces.fragment:continue
                    target=posixpath.normpath(posixpath.join(posixpath.dirname(name),
                        urllib.parse.unquote(pieces.path))) if pieces.path else name
                    oldid=urllib.parse.unquote(pieces.fragment)
                    new=replacements.get((target,oldid))
                    if not new:continue
                    rewritten=urllib.parse.urlunsplit((pieces.scheme,pieces.netloc,
                       pieces.path,pieces.query,urllib.parse.quote(new,safe='-._~')))
                    node.set(field,rewritten);dirty=True;changed_links+=1
            if dirty:self.set(name,root)
        self.report['human_readable_anchor_ids']=len(replacements)
        self.report['rewritten_anchor_links']=changed_links
        self.report['anchor_documents_updated']=changed_docs

    def update_metadata(self):
        metadata=self.opf.find(Q(OPF,'metadata'))
        if metadata is None:raise RuntimeError('EPUB metadata missing')
        def dc(tag,value):
            existing=[x for x in metadata if x.tag==Q(DC,tag)]
            if not existing:E.SubElement(metadata,Q(DC,tag)).text=value
        dc('rights','Copyright © 2026 Swayam Singal. All rights reserved.')
        dc('description','A C++23-first mathematical foundations textbook for graphics, simulation, numerical geometry, uncertainty, and reproducible experiments. Book Two in the From Bits to Research series.')
        for term in ('Mathematics','Computer graphics','Numerical analysis','Computational geometry','C++23','Scientific computing'):
            if not any(x.tag==Q(DC,'subject') and x.text==term for x in metadata):
                E.SubElement(metadata,Q(DC,'subject')).text=term
        # Do not invent a publisher or Book Two ISBN. UUID is valid under
        # EPUB3; reserved commercial identifiers require an actual assignment.
        series=[x for x in metadata if x.get('property')=='belongs-to-collection']
        if not series:
            m=E.SubElement(metadata,Q(OPF,'meta'),id='book02-series',property='belongs-to-collection')
            m.text='From Bits to Research'
            m=E.SubElement(metadata,Q(OPF,'meta'),refines='#book02-series',property='collection-type')
            m.text='series'
            m=E.SubElement(metadata,Q(OPF,'meta'),refines='#book02-series',property='group-position')
            m.text='2'
        desired=[('schema:accessMode','textual'),('schema:accessibilityFeature','MathML'),
                 ('schema:accessibilityFeature','alternativeText'),
                 ('schema:accessibilityFeature','structuralNavigation'),
                 ('schema:accessibilityFeature','tableOfContents'),
                 ('schema:accessibilityFeature','index'),
                 ('schema:accessibilitySummary','Contains semantic MathML expressions, descriptive figure alternatives, numbered figure references, linked glossary and subject index, and keyboard-readable navigation. Some older ereaders may not render MathML faithfully; for precise mathematical typography use the companion PDF.')]
        for attr,text in desired:
            if not any(x.get('property')==attr and norm(x.text)==text for x in metadata):
                m=E.SubElement(metadata,Q(OPF,'meta'),property=attr);m.text=text
        for x in metadata:
            if x.get('property')=='dcterms:modified':
                x.text='2026-11-20T12:00:00Z'
        # Critical: the OPF manifest and spine were edited in memory.
        self.set('EPUB/content.opf',ser(self.opf))
    def sync_mathml_manifest(self):
        """EPUB OPF must declare MathML iff that XHTML actually has MathML.

        Splitting 16 legacy chapters copies manifest properties into new
        fragments. Some now contain no math; EPUBCheck reports OPF-015 for
        the stale declarations (e.g. ch008-p08, ch010-p07/p10, ch016-p07).
        Preserve every equation and update only resource-property tokens.
        """
        added=removed=0
        for item in self.manifest:
            if item.get('media-type')!='application/xhtml+xml':continue
            path=posixpath.normpath(posixpath.join(
                'EPUB',urllib.parse.unquote(item.get('href',''))))
            if path not in self.data:
                raise RuntimeError('Missing XHTML for OPF MathML audit: '+path)
            contains_math=bool(parse(self.data[path]).xpath(
                './/m:math',namespaces=NS))
            props=item.get('properties','').split()
            declared='mathml' in props
            if contains_math and not declared:
                props.append('mathml');added+=1
            elif declared and not contains_math:
                props.remove('mathml');removed+=1
            if not props:item.attrib.pop('properties',None)
            else:item.set('properties',' '.join(dict.fromkeys(props)))
        self.report['mathml_opf_flags_added']=added
        self.report['mathml_opf_flags_removed']=removed

    def validate_original_paragraphs(self):
        """Independent normalized paragraph multiset, chapter by chapter.

        Additive exercises/captions are allowed, but an original paragraph
        must not disappear when its containing section is moved/split.
        """
        verified=0
        with zipfile.ZipFile(self.source) as original:
            for chapter in range(1,17):
                original_number=self.reading_order[chapter-1]
                filename=f'EPUB/text/ch{original_number+5:03}.xhtml'
                document=parse(original.read(filename))
                old=collections.Counter(norm(''.join(el.itertext()))
                    for el in document.iter(Q(X,'p'))
                    if norm(''.join(el.itertext())))
                current=collections.Counter()
                for fragment in self.chapter_paths[chapter]:
                    doc=parse(self.data[fragment])
                    current.update(norm(''.join(el.itertext()))
                        for el in doc.iter(Q(X,'p'))
                        if norm(''.join(el.itertext())))
                lost=old-current
                if lost:
                    raise RuntimeError(
                        f'Original Book Two chapter {chapter} lost '
                        f'{sum(lost.values())} paragraphs; first missing: '
                        +repr(next(iter(lost)))[:130])
                verified+=sum(old.values())
        self.report['original_paragraphs_verified_unchanged']=verified

    def validate_internal(self):
        """Cross-check the whole revised package before emitting a ZIP."""
        failures=[]; xhtmls={};ids={}
        for name,data in self.data.items():
            if not name.endswith(('.xhtml','.ncx','.opf','.svg')):continue
            try:root=parse(data)
            except Exception as e:
                failures.append('Invalid XML '+name+': '+str(e));continue
            if name.endswith('.xhtml'):
                xhtmls[name]=root
                anchors=collections.Counter(x.get('id') for x in root.iter() if x.get('id'))
                for ident,count in anchors.items():
                    if count>1:failures.append(f'Duplicate ID within {name}: {ident}')
                ids[name]=set(anchors)
        broken=[];links=0
        for name,root in xhtmls.items():
            for node in root.iter():
                for attr in ('href','src'):
                    href=node.get(attr)
                    if not href or href.startswith(('http:','https:','mailto:','data:','tel:','//')):continue
                    parsed=urllib.parse.urlsplit(href)
                    target=posixpath.normpath(posixpath.join(posixpath.dirname(name),
                        urllib.parse.unquote(parsed.path))) if parsed.path else name
                    links+=1
                    if target not in self.data:
                        broken.append(f'{name}: missing resource {href} => {target}')
                    elif parsed.fragment and target in ids:
                        frag=urllib.parse.unquote(parsed.fragment)
                        if frag not in ids[target]:broken.append(f'{name}: missing anchor {href}')
        # NCX references also must continue to exist in the revised archive.
        ncx=parse(self.data['EPUB/toc.ncx'])
        for item in ncx.xpath('.//ncx:content',namespaces=NS):
            uri=urllib.parse.urlsplit(item.get('src') or '')
            if uri.scheme:continue
            target=posixpath.normpath(posixpath.join('EPUB',urllib.parse.unquote(uri.path)))
            if target not in self.data:broken.append('NCX: missing '+target)
            elif uri.fragment and target in ids and urllib.parse.unquote(uri.fragment) not in ids[target]:
                broken.append('NCX: missing anchor '+item.get('src'))
        if broken:failures.extend(broken[:80])
        opf=parse(self.data['EPUB/content.opf'])
        manifest=opf.find(Q(OPF,'manifest'));spine=opf.find(Q(OPF,'spine'))
        mapping={i.get('id'):i for i in manifest}
        for item in manifest:
            path=posixpath.normpath(posixpath.join('EPUB',urllib.parse.unquote(item.get('href',''))))
            if path not in self.data:failures.append('OPF manifest missing asset '+path)
        for item in spine:
            if item.get('idref') not in mapping:failures.append('Broken OPF spine '+str(item.get('idref')))
        for item in manifest:
            if item.get('media-type')!='application/xhtml+xml':continue
            path=posixpath.normpath(posixpath.join(
                'EPUB',urllib.parse.unquote(item.get('href',''))))
            doc=xhtmls.get(path)
            if doc is None:continue
            declared='mathml' in item.get('properties','').split()
            actual=bool(doc.xpath('.//m:math',namespaces=NS))
            if declared!=actual:
                failures.append(f'Invalid OPF mathml property in {path}: '
                                f'declared={declared}, has_math={actual}')
        nav=xhtmls.get('EPUB/nav.xhtml')
        if nav is None:failures.append('Missing EPUB3 nav.xhtml')
        else:
            landmark=[n for n in nav.iter(Q(X,'nav'))
                      if 'landmarks' in n.get(Q(EPUB,'type'),'').split()]
            if len(landmark)!=1:failures.append(
                f'Exactly one landmarks nav required, found {len(landmark)}')
            if landmark and landmark[0].get('role')=='doc-landmarks':
                failures.append('Invalid ARIA doc-landmarks role')
            spine_paths={mapping[ref.get('idref')].get('href'):i
                         for i,ref in enumerate(spine)
                         if ref.get('idref') in mapping}
            tops=nav.xpath('.//x:nav[@id="toc"]/x:ol/x:li/x:a',
                           namespaces=NS)
            toc_positions=[]
            for anchor in tops:
                href=urllib.parse.unquote(
                    urllib.parse.urlsplit(anchor.get('href','')).path)
                if href not in spine_paths:failures.append(
                    'TOC target absent from OPF spine: '+href)
                else:toc_positions.append(spine_paths[href])
            if toc_positions!=sorted(toc_positions):
                failures.append('EPUB navigation is not in spine reading order')
        count_math=sum(len(r.xpath('.//m:math',namespaces=NS)) for r in xhtmls.values())
        count_code=sum(len(r.xpath('.//x:pre',namespaces=NS)) for r in xhtmls.values())
        count_image=sum(len(r.xpath('.//x:img',namespaces=NS)) for r in xhtmls.values())
        if count_math!=self.math_before:failures.append(f'MathML lost {self.math_before} -> {count_math}')
        if count_code<self.code_before:failures.append(f'Code pre lost {self.code_before} -> {count_code}')
        # At least the original 65 chapter images; the cover is outside that.
        if count_image<self.fig_before:failures.append(f'Images lost {self.fig_before} -> {count_image}')
        oversized={n:len(self.data[n]) for n in xhtmls if re.fullmatch(r'EPUB/text/ch0(?:0[6-9]|1\d|2[01])(?:-p\d\d)?\.xhtml',n) and len(self.data[n])>67000}
        if oversized:failures.append('Oversized split XHTML '+str(oversized))
        if 'ns0:' in self.data['EPUB/nav.xhtml'].decode('utf8') or 'ns0:' in self.data['EPUB/toc.ncx'].decode('utf8'):
            failures.append('Unclean XML namespace prefix in TOC')
        cover=parse(self.data['EPUB/text/cover.xhtml'])
        image=cover.xpath('.//x:img',namespaces=NS)
        if len(image)!=1 or not image[0].get('alt'):failures.append('No visible cover')
        self.report.update({
            'original_mathml':self.math_before,'final_mathml':count_math,
            'original_code_blocks':self.code_before,'final_code_blocks':count_code,
            'original_chapter_images':self.fig_before,'final_total_images':count_image,
            'xhtml_files':len(xhtmls),'max_xhtml_kib':round(max(map(len,(self.data[n] for n in xhtmls)))/1024,1),
            'checked_local_links':links,'broken_internal_links':len(broken),
            'package_files':len(self.data),'chapters':len(self.chapter_paths),
            'independent_epubcheck':'Not run in container; EPUBCheck 5.4.0 on Mac remains required',
            'new_publication_metadata':'description, rights, subjects, series 2, accessibility MathML summary; no ISBN/publisher invented'
        })
        if failures:
            self.report['errors']=failures
            raise RuntimeError('\n'.join(failures[:35]))
    def save(self,output):
        self.sync_mathml_manifest()
        self.set('EPUB/content.opf',ser(self.opf))
        self.validate_original_paragraphs()
        self.validate_internal()
        path=pathlib.Path(output);path.parent.mkdir(parents=True,exist_ok=True)
        temp=path.with_suffix('.epub.tmp')
        try:
            with zipfile.ZipFile(temp,'w') as out:
                for name in self.names:
                    if name not in self.data:continue
                    if name=='mimetype':
                        out.writestr('mimetype',self.data[name],compress_type=zipfile.ZIP_STORED)
                    else:
                        meta=copy.copy(self.originals[name]) if name in self.originals else zipfile.ZipInfo(name)
                        meta.compress_type=zipfile.ZIP_DEFLATED
                        out.writestr(meta,self.data[name])
            with zipfile.ZipFile(temp) as z:
                if z.testzip() is not None:raise RuntimeError('ZIP CRC failure')
                if z.namelist()[0]!='mimetype' or z.getinfo('mimetype').compress_type!=zipfile.ZIP_STORED:
                    raise RuntimeError('EPUB mimetype was not first and uncompressed')
            os.replace(temp,path)
        finally:
            temp.unlink(missing_ok=True)
        self.report['output']=str(path)
        self.report['output_size_mib']=round(path.stat().st_size/1024**2,2)
        self.report['sha256']=hashlib.sha256(path.read_bytes()).hexdigest()
        return self.report


class Editor(Editor):
    def split_extra_long_backmatter(self):
        """Retain the full cross-stage code attachments, split large XHTML."""
        path='EPUB/text/ch023.xhtml'
        root=self.get_root(path)
        book_id,nodes=self._chapter_nodes(17,root)
        groups=[];current=[];size=0
        for node in nodes:
            estimate=len(E.tostring(node))
            if current and size+estimate>50000:
                groups.append(current);current=[];size=0
            current.append(node);size+=estimate
        if current:groups.append(current)
        parts=[]
        for i,nodes in enumerate(groups,1):
            destination=path if i==1 else path.replace('.xhtml',f'-p{i:02}.xhtml')
            nr=copy.deepcopy(root); bb=body(nr)
            for c in list(bb):bb.remove(c)
            outer=el('section',id=book_id) if i==1 else el('section',**{'class':'cross-reference-continuation'})
            for n in nodes:outer.append(n)
            bb.append(outer)
            self.set(destination,nr)
            if i>1:
                self.add_item(f'ch023_part{i:02}',destination[len('EPUB/'):],
                               'application/xhtml+xml',spine=False)
            parts.append(destination)
        original_spine=[x for x in self.spine if x.get('idref')=='ch023_xhtml']
        if len(original_spine)!=1:raise RuntimeError('Cross-stage reference spine missing')
        at=self.spine.index(original_spine[0])+1
        for p in parts[1:]:
            item=self.href_item[p[len('EPUB/'):]]
            self.spine.insert(at,E.Element(Q(OPF,'itemref'),idref=item.get('id')))
            at+=1
        self.extra_split_mapping={path:parts}
        self.report['archival_source_attachments_embedded']=3
        self.report['cross_reference_fragments']=len(parts)
        # The subject index is 60+ KB. Divide at the alphabet boundary,
        # retaining all locators and accessible original anchor IDs.
        name='EPUB/text/book02-index.xhtml'
        root=self.get_root(name); dd=root.xpath('.//x:dl',namespaces=NS)
        if len(dd)!=1:raise RuntimeError('Book Two index structure differs')
        listing=dd[0]
        pairs=list(listing)
        if len(pairs)%2:raise RuntimeError('Unexpected index DT/DD sequence')
        cutoff=next((j for j in range(0,len(pairs),2)
                     if norm(''.join(pairs[j].itertext())).casefold() >= 'm'),len(pairs))
        if cutoff==0 or cutoff==len(pairs):raise RuntimeError('Cannot split alphabetical index')
        second=el('section',id='book02-subject-index-m-z')
        second.append(el('h1','Subject Index · M–Z'))
        second_list=el('dl',**{'class':'book02-index'})
        for node in pairs[cutoff:]:
            listing.remove(node);second_list.append(node)
        second.append(second_list)
        extra='EPUB/text/book02-index-m-z.xhtml'
        self.set(name,root)
        self.set(extra,fresh_page('Subject Index M–Z',[second]))
        new_item=self.add_item('book02_index_m_z','text/book02-index-m-z.xhtml',
                               'application/xhtml+xml',spine=False)
        old_item=[x for x in self.spine if x.get('idref')=='book02_index']
        if len(old_item)!=1:raise RuntimeError('Original index missing in spine')
        self.spine.insert(self.spine.index(old_item[0])+1,
                          E.Element(Q(OPF,'itemref'),idref=new_item.get('id')))
        self.report['subject_index_split_pages']=2

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--input',type=pathlib.Path,required=True,
                   help='Existing complete, cover-visible Book Two EPUB to finish')
    p.add_argument('--output',type=pathlib.Path,required=True,
                   help='Separate output EPUB path (never overwrite the source)')
    p.add_argument('--report',type=pathlib.Path,
                   help='Optional JSON validation/provenance report')
    a=p.parse_args()
    if pathlib.Path(a.input).resolve()==pathlib.Path(a.output).resolve():
        p.error('Input and output must be different files')
    editor=Editor(a.input)
    print('ORIGINAL',editor.math_before,'math',editor.code_before,'pre',editor.fig_before,'images',flush=True)
    editor.rewrite_chapters()
    print('CHAPTERS_SPLIT',sum(len(x) for x in editor.chapter_paths.values()),flush=True)
    editor.add_captions_and_math_access()
    print('CAPTIONS',len(editor.figure_records),'EQUATIONS',len(editor.equation_records),flush=True)
    editor.optimize_rich_svg()
    editor.fix_css()
    editor.fix_crossbook_navigation()
    editor.fix_reader_map_promises()
    editor.add_publication_pages()
    editor.add_workbook()
    editor.add_index()
    editor.split_extra_long_backmatter()
    editor.relink_all()
    editor.clean_namespace_and_nav()
    editor.canonicalize_generated_anchors()
    editor.complete_math_accessibility()
    editor.update_metadata()
    print('VALIDATING',flush=True)
    report=editor.save(a.output)
    if a.report is not None:
        a.report.parent.mkdir(parents=True,exist_ok=True)
        a.report.write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')
    print('BOOK02_PUBLICATION_REVISION_OK',json.dumps({k:v for k,v in report.items()
         if k in ('output','output_size_mib','sha256','chapters','xhtml_files','max_xhtml_kib','checked_local_links','broken_internal_links','figure_captions','equations_numbered','new_practice_exercises','subject_index_terms','rich_svgs_converted_to_png')},ensure_ascii=False),flush=True)

if __name__=='__main__':main()
