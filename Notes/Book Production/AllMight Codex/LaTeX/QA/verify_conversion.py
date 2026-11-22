#!/usr/bin/env python3
"""Independent source/hash/node/code/table QA for an AllMight LaTeX generation."""
import argparse, collections, hashlib, json, re, subprocess
from pathlib import Path

V=Path('/Users/swayamsingal/Library/Mobile Documents/iCloud~md~obsidian/Documents/Master')
R=V/'Book Production/AllMight Codex'
A=Path('/Users/swayamsingal/.codex/source-cache/allmight-book-production-2026-11-15')

def sha(b): return hashlib.sha256(b).hexdigest()
def pandoc_verbatim_payload(body):
    """Return the exact payload Pandoc exposes between verbatim delimiters.

    Pandoc's LaTeX writer ensures one LF before ``\\end{verbatim}``. That LF
    is the environment separator; when the source body already ends in LF,
    that one final LF is indistinguishable from the separator in TeX output.
    Remove exactly that one LF, never all trailing whitespace/newlines.
    """
    return body[:-1] if body.endswith('\n') else body
def walk(x):
    if isinstance(x,dict):
        yield x
        for v in x.values(): yield from walk(v)
    elif isinstance(x,list):
        for v in x: yield from walk(v)
def ast_for(row):
    if row.get('reader_ast'):
        return json.loads(Path(row['reader_ast']).read_text())
    # Match the converter's reader extensions exactly; the double-backslash
    # math extension changes source Math-node inventory in legacy notes.
    p=subprocess.run(['pandoc','-f','markdown+wikilinks_title_after_pipe+tex_math_single_backslash+tex_math_double_backslash','-t','json'],input=Path(row['source']).read_bytes(),stdout=subprocess.PIPE,stderr=subprocess.PIPE)
    if p.returncode: raise RuntimeError(p.stderr.decode(errors='replace'))
    return json.loads(p.stdout)
def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--generation',default='.generation-jvekxwqt'); args=ap.parse_args()
    gen=R/'LaTeX'/args.generation; man=json.loads((gen/'conversion-manifest.json').read_text())
    rows=man['rows']; issues=[]; total=collections.Counter(); source_chapters=0; ref=0; source_math=0; transformed_math=0; code_blocks=0; tables=0; code_checked=0; transformed_code_checked=0; verbatim_code_checked=0; table_tex=0; transformed_ast_failures=0; code_output_failures=0
    exceptions=collections.Counter((e.get('kind') or 'unknown') for e in man.get('exceptions',[]))
    blank_padded_restorations=exceptions.get('blank-padded-display-math-restored',0)
    spaced_dollar_restorations=exceptions.get('spaced-dollar-math-restored',0)
    restored_by_source=collections.Counter(e.get('source') for e in man.get('exceptions',[]) if e.get('kind')=='spaced-dollar-math-restored')
    chapter_hierarchy=[]
    for i,row in enumerate(rows):
        src=Path(row['source']); texpath=gen/row['volume']/'content'/(Path(row['tex']).name)
        try:
            raw=src.read_bytes(); source_ok=sha(raw)==row['source_sha256']
            if not source_ok: issues.append({'row':i,'kind':'source_sha256_mismatch','source':str(src)})
            ast=ast_for(row); nodes=list(walk(ast)); counts=collections.Counter(n.get('t') for n in nodes if isinstance(n,dict) and 't' in n)
            for k,v in counts.items(): total[k]+=v
            if row['kind']=='chapter': source_chapters+=1
            else: ref+=1
            source_math += counts['Math']; code_blocks += counts['CodeBlock']; tables += counts['Table']
            got=[sha(n['c'][1].encode()) for n in nodes if n.get('t')=='CodeBlock' and 'mermaid' not in (n.get('c',[['',[]]])[0][1] if isinstance(n.get('c'),list) and n['c'] and isinstance(n['c'][0],list) else '')]
            # Independent body hashes must equal converter's source code ledger, in traversal order.
            code_ok=got==row.get('code_sha256',[]); code_checked+=len(got)
            if not code_ok: issues.append({'row':i,'kind':'code_body_hash_mismatch','source':str(src),'expected_count':len(got),'ledger_count':len(row.get('code_sha256',[]))})
            if not texpath.is_file(): issues.append({'row':i,'kind':'tex_missing','path':str(texpath)}); continue
            tex=texpath.read_text(errors='replace')
            # Manifest absolute paths may point through the currently active
            # symlink while a candidate is staged. Prefer the AST inside the
            # generation under audit; only fall back to its manifest path if
            # the staged tree lacks the expected file.
            transformed_path=gen/row['volume']/'AST'/(texpath.stem+'.json')
            if not transformed_path.is_file(): transformed_path=Path(row.get('transformed_ast',''))
            if transformed_path.is_file():
                transformed_bytes=transformed_path.read_bytes()
                if sha(transformed_bytes)!=row.get('transformed_ast_sha256'):
                    issues.append({'row':i,'kind':'transformed_ast_sha256_mismatch','path':str(transformed_path)}); transformed_ast_failures+=1
                transformed=json.loads(transformed_bytes); transnodes=list(walk(transformed)); transcounts=collections.Counter(n.get('t') for n in transnodes if isinstance(n,dict) and 't' in n); transformed_math+=transcounts['Math']
                expected_trans_codes=[sha(n['c'][1].encode()) for n in transnodes if n.get('t')=='CodeBlock' and 'mermaid' not in (n.get('c',[['',[]]])[0][1] if isinstance(n.get('c'),list) and n['c'] and isinstance(n['c'][0],list) else '')]
                if expected_trans_codes!=row.get('code_sha256',[]): issues.append({'row':i,'kind':'transformed_ast_code_body_hash_mismatch','source':str(src)}); transformed_ast_failures+=1
                # Pandoc's no-highlight verbatim blocks should preserve every non-Mermaid source code body.
                expected_bodies=[n['c'][1] for n in transnodes if n.get('t')=='CodeBlock' and 'mermaid' not in (n.get('c',[['',[]]])[0][1] if isinstance(n.get('c'),list) and n['c'] and isinstance(n['c'][0],list) else '')]
                rendered_bodies=re.findall(r'\\begin\{verbatim\}\n(.*?)\\end\{verbatim\}',tex,re.S)
                # The writer adds/uses one LF before the closing delimiter.
                # Capture it, then account for exactly that separator byte;
                # retain every other trailing LF, tab, space, and blank line.
                expected_verbatim=[pandoc_verbatim_payload(body) for body in expected_bodies]
                actual_verbatim=[pandoc_verbatim_payload(body) for body in rendered_bodies]
                if len(actual_verbatim)!=len(expected_verbatim) or any(a!=b for a,b in zip(expected_verbatim,actual_verbatim)):
                    code_output_failures+=1
                    mismatch=next(((j,a,b) for j,(a,b) in enumerate(zip(expected_verbatim,actual_verbatim)) if a!=b),None)
                    issue={'row':i,'kind':'tex_verbatim_code_body_mismatch','source':str(src),'expected_blocks':len(expected_bodies),'rendered_blocks':len(rendered_bodies)}
                    if mismatch:
                        j,a,b=mismatch; offset=next((k for k,(x,y) in enumerate(zip(a,b)) if x!=y),min(len(a),len(b)))
                        issue.update({'block':j,'first_difference_offset':offset,'expected_length':len(a),'rendered_length':len(b)})
                    issues.append(issue)
                else: verbatim_code_checked+=len(expected_bodies)
            else:
                issues.append({'row':i,'kind':'transformed_ast_missing','source':str(src)}); transformed_ast_failures+=1
            table_tex+=len(re.findall(r'\\begin\{longtable\}',tex))
            if row['kind']=='chapter':
                generated=1 if re.search(r'\\chapter\{',tex) else 0
                if generated!=1: issues.append({'row':i,'kind':'numbered_chapter_heading_count','count':generated,'source':str(src)})
                # Source H1 headings render as extra chapter commands: record for focused hierarchy correction.
                actual=len(re.findall(r'\\chapter\{',tex))
                if actual!=1: chapter_hierarchy.append({'source':str(src),'chapter_commands':actual})
            # TeX remains source-faithful in significant table count; card conversion is not allowed.
            if len(re.findall(r'\\begin\{longtable\}',tex)) != counts['Table']:
                issues.append({'row':i,'kind':'table_tex_count_mismatch','source':str(src),'ast_tables':counts['Table'],'tex_longtables':len(re.findall(r'\\begin\{longtable\}',tex))})
        except Exception as e:
            issues.append({'row':i,'kind':'audit_exception','source':str(src),'error':str(e)[:300]})
    expected={'rows':568,'chapters':129,'references':439,'Table':512,'CodeBlock':6070}
    actual={'rows':len(rows),'chapters':source_chapters,'references':ref,'Table':tables,'CodeBlock':code_blocks}
    global_issues=[]
    for k,v in expected.items():
        if actual[k]!=v: global_issues.append({'kind':'inventory_count_mismatch','field':k,'expected':v,'actual':actual[k]})
    if table_tex!=512: global_issues.append({'kind':'longtable_total_mismatch','expected':512,'actual':table_tex})
    manifest_math=sum(r.get('node_counts',{}).get('Math',0) for r in rows)
    expected_manifest_math=source_math+blank_padded_restorations
    expected_transformed_math=manifest_math+spaced_dollar_restorations
    math_reconciled=(manifest_math==expected_manifest_math and transformed_math==expected_transformed_math
                     and transformed_math==source_math+blank_padded_restorations+spaced_dollar_restorations)
    if not math_reconciled:
        global_issues.append({'kind':'math_count_reconciliation','source_math':source_math,
                              'blank_padded_restorations':blank_padded_restorations,
                              'expected_manifest_math':expected_manifest_math,'manifest_math':manifest_math,
                              'spaced_dollar_restorations':spaced_dollar_restorations,
                              'expected_transformed_math':expected_transformed_math,
                              'transformed_math':transformed_math})
    report={'schema':'allmight-source-fidelity-v1','generation_id':man.get('generation_id'),'stage_path':str(gen),'status':'pass' if not issues and not global_issues and not chapter_hierarchy else 'fail','scope':'frozen pre-promotion candidate; source hashes, independent AST inventories/code hashes/table output counts','source_inventory':actual,'expected_inventory':expected,'independent_source_ast_node_counts':dict(total),'independent_native_math_node_count':source_math,'manifest_math_count_sum':manifest_math,'blank_padded_display_math_restorations':blank_padded_restorations,'spaced_dollar_math_restorations':spaced_dollar_restorations,'expected_manifest_math_from_source_and_ledger':expected_manifest_math,'expected_transformed_math_from_manifest_and_ledger':expected_transformed_math,'math_count_reconciliation':'pass' if math_reconciled else 'fail','source_code_body_hashes_checked':code_checked,'transformed_ast_math_node_count':transformed_math,'transformed_ast_code_body_hashes_checked':code_checked,'tex_verbatim_code_bodies_checked':verbatim_code_checked,'transformed_ast_failures':transformed_ast_failures,'code_output_failures':code_output_failures,'longtable_count':table_tex,'converter_exception_counts':dict(exceptions),'chapter_hierarchy_anomalies':chapter_hierarchy,'issues':issues,'global_issues':global_issues,'limitations':['Exact formula text provenance remains separately reviewable in converter exception rows.','PDF compilation, links, font embedding, diagram layout, raster review are outside this source audit.']}
    out=R/'LaTeX/QA/source-fidelity.json'; out.write_text(json.dumps(report,indent=2,ensure_ascii=False)+'\n')
    final={'schema':'allmight-final-verification-v1','generation_id':man.get('generation_id'),'status':'incomplete','gates':{'source_hashes_tables_and_code_bodies':'pass' if not issues and not global_issues else 'fail','129_chapter_hierarchy_and_439_companion_coverage':'fail' if chapter_hierarchy else 'pass','math_normalization_ledger':'pass' if math_reconciled else 'fail','6_volume_pdf_builds':'incomplete','pdf_font_embedding':'incomplete','pdf_links_and_named_destinations':'incomplete','render_review':'incomplete','diagram_121_layout_gate':'incomplete'},'evidence':{'source_fidelity_report':str(out),'candidate_stage':str(gen),'chapter_hierarchy_anomaly_count':len(chapter_hierarchy),'independent_source_math_ast_nodes':source_math,'manifest_math_count_sum':manifest_math,'blank_padded_display_math_restorations':blank_padded_restorations,'spaced_dollar_math_restorations':spaced_dollar_restorations,'expected_manifest_math_from_source_and_ledger':expected_manifest_math,'table_ast_count':tables,'tex_longtable_count':table_tex,'transformed_math_node_count':transformed_math,'expected_transformed_math_from_ledger':expected_transformed_math,'code_body_output_failures':code_output_failures},'note':'Not a completion claim; candidate is pre-promotion and PDF/diagram gates remain open.'}
    (R/'LaTeX/QA/final-verification.json').write_text(json.dumps(final,indent=2)+'\n')
    print(json.dumps({'status':report['status'],'rows':actual,'sourceMath':source_math,'codeHashesChecked':code_checked,'longtables':table_tex,'issueCount':len(issues)+len(global_issues),'hierarchyAnomalies':len(chapter_hierarchy),'reports':[str(out),str(R/'LaTeX/QA/final-verification.json')]},indent=2))
if __name__=='__main__': main()
