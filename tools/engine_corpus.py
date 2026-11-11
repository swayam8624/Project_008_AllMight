#!/usr/bin/env python3
"""Import the audited video and book corpus into the local notes pipeline."""
from __future__ import annotations
import argparse, json, sys
from pathlib import Path

ROOT=Path.home()/'.codex/source-cache/game-engine-2026-11-05'
STATE=Path.home()/'.codex/source-cache/engine-corpus-2026-11-07'
sys.path.insert(0,str(Path(__file__).resolve().parent))
import notes_pipeline as np

def derive_spans(units, target_words=150, source_id=''):
    """Group transcript cues without discarding or rewriting any constituent cue."""
    spans=[]; group=[]; words=0
    def emit(items):
        if not items:return
        locs=[u['locator'] for u in items]
        spans.append({'unit_id':'span-'+str(len(spans)+1),'text':' '.join(u.get('text','') for u in items if u.get('text','')).strip(),
          'locator':{'kind':'subtitle_cue','format':locs[0].get('format'),'cue_start':locs[0].get('cue_index'),'cue_end':locs[-1].get('cue_index'),'start_timestamp':locs[0].get('start_timestamp'),'end_timestamp':locs[-1].get('end_timestamp'),'start_seconds':locs[0].get('start_seconds'),'end_seconds':locs[-1].get('end_seconds'),'constituent_unit_ids':[u['unit_id'] for u in items]},
          'raw_text':' '.join(u.get('raw_text',u.get('text','')) for u in items).strip()})
    for unit in units:
        group.append(unit); words+=len((unit.get('text') or '').split())
        if words>=target_words: emit(group); group=[]; words=0
    emit(group)
    expected=[u['unit_id'] for u in units]
    traced=[x for s in spans for x in s['locator']['constituent_unit_ids']]
    if traced!=expected: raise ValueError('subtitle span trace does not preserve cue IDs exactly once and in order')
    return spans

def asr_actual_path(video_id, fallback):
    # Prefer the timestamped VTT recorded by legacy batch receipts over flattened text.
    for receipt in ROOT.glob('batches/batch-*/generated-transcript.json'):
        try:
            data=json.loads(receipt.read_text())
            if isinstance(data,list): rows=data
            else: rows=[data]
            for row in rows:
                if row.get('video_id',row.get('id'))==video_id:
                    p=Path(row.get('transcript_file',''))
                    if p.is_file() and p.suffix.lower() in ('.vtt','.srt'): return p
        except (OSError,ValueError): pass
    return Path(fallback)

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--audit',default=str(ROOT/'work/source-audit.json')); ap.add_argument('--state-dir',default=str(STATE)); ap.add_argument('--pdf-assignment',default=str(ROOT/'batches/batch-015/pdf-assignment.json')); ap.add_argument('--kohi-pdf',default='/Users/swayamsingal/Downloads/Kohi_Game_Engine_Source_Checked_Technical_Companion_2026.pdf'); ap.add_argument('--manifest'); a=ap.parse_args()
    state=Path(a.state_dir); np.ensure_state_dirs(state); mf=Path(a.manifest) if a.manifest else state/'corpus-manifest.json'
    m=json.loads(mf.read_text()) if mf.exists() else {'schema_version':1,'sources':{},'videos':{},'books':{}}
    audit=json.loads(Path(a.audit).read_text()); items=audit['items']
    for key,rec in list(m['sources'].items()):
        if ':local_asr:' in key and Path(rec.get('path','')).suffix.lower()=='.txt':
            del m['sources'][key]
    # Put the known, unmerged batch-016 ASR first so the pilot can use it immediately.
    items=sorted(items,key=lambda x:(x['id']!='cCEmwjfHIkY',x['id']))
    for i,item in enumerate(items,1):
        vid=item['id']; url=f'https://www.youtube.com/watch?v={vid}'; ids=[]
        candidates=[]
        for typ,key,hkey in [('youtube_caption','caption_files','caption_sha256'),('local_asr','generated_transcript_files','generated_transcript_sha256')]:
            for j,rawpath in enumerate(item.get(key,[])):
                p=Path(rawpath)
                if typ=='local_asr': p=asr_actual_path(vid,p)
                if p.is_file(): candidates.append((typ,p,item.get(hkey,[])[j] if j<len(item.get(hkey,[])) else None))
        # ASR can be in legacy receipts but absent from the merged audit.
        if vid=='cCEmwjfHIkY' and not any(t=='local_asr' for t,_,_ in candidates):
            for receipt in ROOT.glob('batches/batch-016/**/*'):
                if receipt.is_file() and receipt.name.endswith(('.vtt','.srt')) and vid in receipt.name: candidates.append(('local_asr',receipt,None))
        # When the audit also lists a flattened convenience TXT, retain the
        # timestamped original VTT as the canonical transcript evidence.
        if any(t=='local_asr' and p.suffix.lower() in ('.vtt','.srt') for t,p,_ in candidates):
            candidates=[x for x in candidates if x[0]!='local_asr' or x[1].suffix.lower() in ('.vtt','.srt')]
        for typ,p,expected in candidates:
            actual=np.sha256_file(p)
            if expected and (typ=='youtube_caption' or p.suffix.lower() not in ('.vtt','.srt')) and actual!=expected:
                raise RuntimeError(f'hash mismatch: {p}')
            sig=f'{vid}:{typ}:{actual}'; rec=m['sources'].get(sig)
            if not rec:
                meta=np.import_source(p,state,f'youtube:{vid}:{typ}',item['title'],kind=typ,extra={'video_id':vid,'video_url':url,'channel':item['channel'],'reading_status':item.get('reading_status'),'caption_status':item.get('caption_status'),'audit_path':str(Path(a.audit).resolve()),'original_file_sha256':actual,'generated_transcript_provenance':item.get('generated_transcript_provenance') if typ=='local_asr' else None})
                rec={'source_id':meta['source_id'],'sha256':actual,'path':str(p),'kind':typ,'unit_count':meta['unit_count']}; m['sources'][sig]=rec
                if p.suffix.lower() in ('.vtt','.srt'):
                    _,units=np.load_source(state,meta['source_id']); spans=derive_spans(units,150,meta['source_id'])
                    dpath=state/'sources'/meta['source_id']/'subtitle-spans.json'; np.atomic_json(dpath,spans); rec['span_file']=str(dpath); rec['span_count']=len(spans)
            ids.append(rec['source_id'])
        m['videos'][vid]={'video_id':vid,'title':item['title'],'channel':item['channel'],'url':url,'source_ids':ids,'acquisition_needed':not bool(ids),'legacy_reading_status':item.get('reading_status'),'caption_status':item.get('caption_status'),'review_records':item.get('review_records',[])}
        if i%15==0: np.atomic_json(mf,m)
    inventory=[{'id':x['id'],'title':x['title'],'url':f"https://www.youtube.com/watch?v={x['id']}",'index':i,'channel':x['channel']} for i,x in enumerate(json.loads(Path(a.audit).read_text())['items'],1)]
    np.atomic_json(state/'inventory'/'engine-all.json',{'playlist_id':'engine-all','source':'source-audit.json','entries':inventory,'entry_count':len(inventory)})
    assignment=json.loads(Path(a.pdf_assignment).read_text()); seen=set()
    for row in assignment:
        path=Path(row['file']).expanduser(); expected=row.get('pdf_sha256');
        if str(path) in seen or not path.is_file(): continue
        seen.add(str(path)); actual=np.sha256_file(path)
        if expected and actual!=expected: raise RuntimeError(f'book hash mismatch: {path}')
        key='book:'+actual
        if key not in m['books']:
            meta=np.import_source(path,state,'book:'+path.name,row.get('book',path.stem),kind='secondary_pdf',extra={'assignment_record':row,'original_file_sha256':actual})
            m['books'][key]={'source_id':meta['source_id'],'sha256':actual,'path':str(path),'title':row.get('book',path.stem),'unit_count':meta['unit_count'],'page_count':meta['format_metadata']['total_pdf_pages']}
    kp=Path(a.kohi_pdf)
    if kp.is_file():
        actual=np.sha256_file(kp); key='book:'+actual
        if key not in m['books']:
            meta=np.import_source(kp,state,'book:'+kp.name,kp.stem,kind='secondary_pdf',extra={'supplied_companion':True,'original_file_sha256':actual})
            m['books'][key]={'source_id':meta['source_id'],'sha256':actual,'path':str(kp),'title':kp.stem,'unit_count':meta['unit_count'],'page_count':meta['format_metadata']['total_pdf_pages']}
    np.atomic_json(mf,m)
    vlist=[m['videos'][x['id']] for x in json.loads(Path(a.audit).read_text())['items']]
    blist=[{'title':b['title'],'source_id':b['source_id'],'total_pdf_pages':b['page_count'],'pdf_path':b['path']} for b in m['books'].values()]
    np.atomic_json(state/'source-index.json',{'schema_version':1,'videos':vlist,'books':blist,'source_audit':str(Path(a.audit).resolve())})
    print(json.dumps({'videos':len(vlist),'saved_video_files':sum(bool(v['source_ids']) for v in vlist),'needs_acquisition':sum(v['acquisition_needed'] for v in vlist),'video_sources':len(m['sources']),'books':blist,'manifest':str(mf)},indent=2))
if __name__=='__main__': main()
