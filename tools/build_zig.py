"""Isolated Windows x64 developer build, no deployment or game execution.

Zig/MinGW provides a portable alternative to the existing MSVC project.
External inputs must be supplied explicitly; no dependency is downloaded here.
"""
from __future__ import annotations
import argparse, concurrent.futures, hashlib, json, os, shutil, struct, subprocess, time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
TARGET_SHA="324c51e9ea3531758842e16c69e6cddbefbb226c5b675c3d6e60111646c2e98c"
def sha(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def save(p,x): p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(x,indent=2)+'\n',encoding='utf-8')
class PE:
 def __init__(self,p):
  self.d=p.read_bytes();self.nt=struct.unpack_from('<I',self.d,0x3c)[0];n=self.nt;assert self.d[n:n+4]==b'PE\0\0';assert self.u('<H',n+4)[0]==0x8664
  self.opt=n+24;assert self.u('<H',self.opt)[0]==0x20b
  self.sections=[]
  for i in range(self.u('<H',n+6)[0]):
   o=self.opt+self.u('<H',n+20)[0]+i*40;v,r,s,b=self.u('<IIII',o+8);self.sections.append((r,s,b))
 def u(self,f,o):return struct.unpack_from(f,self.d,o)
 def off(self,r):
  if r<self.u('<I',self.opt+60)[0]:return r
  for start,size,raw in self.sections:
   if start<=r<start+size:return raw+r-start
  raise ValueError('unmapped RVA')
 def st(self,r):o=self.off(r);return self.d[o:self.d.index(0,o)].decode('ascii')
 def exports(self):
  r,s=self.u('<II',self.opt+112);o=self.off(r);b,c,n,f,ns,ords=self.u('<IIIIII',o+16);names={}
  for i in range(n):names[self.u('<H',self.off(ords)+2*i)[0]]=self.st(self.u('<I',self.off(ns)+4*i)[0])
  out=[]
  for i in range(c):
   addr=self.u('<I',self.off(f)+4*i)[0]
   if addr:out.append(dict(name=names.get(i),ordinal=b+i,forwarder=self.st(addr) if r<=addr<r+s else None))
  return out
 def imports(self):
  r,s=self.u('<II',self.opt+120);out=[]
  for i in range(s//20):
   values=self.u('<IIIII',self.off(r)+20*i)
   if not any(values):break
   out.append(self.st(values[3]))
  return out

def main():
 ap=argparse.ArgumentParser()
 for name in ['zig','minhook','asmjit','capstone','gamecore','out']:ap.add_argument('--'+name,type=Path,required=True)
 ap.add_argument('--jobs',type=int,default=4);ap.add_argument('--skip-native',action='store_true');a=ap.parse_args()
 for k in ['zig','minhook','asmjit','capstone','gamecore']:setattr(a,k,getattr(a,k).resolve(strict=True))
 a.out=a.out.resolve();assert not a.out.is_relative_to(ROOT);a.out.mkdir(parents=True,exist_ok=True)
 assert sha(a.gamecore)==TARGET_SHA,'GameCore build mismatch'
 inputs=[p for p in ROOT.rglob('*') if p.is_file() and '.git' not in p.parts and p.suffix.lower() in ['.cpp','.h','.c','.s','.asm','.py','.vcxproj']]
 pins=[dict(path=str(p.relative_to(ROOT)),sha256=sha(p),bytes=p.stat().st_size) for p in sorted(inputs)]
 core=PE(a.gamecore);exports=core.exports();assert len(exports)==157
 stage=a.out/'source';stage.mkdir(exist_ok=True)
 for p in ROOT.iterdir():
  if p.suffix.lower() in ('.cpp','.h'):shutil.copyfile(p,stage/p.name)
 shutil.copytree(ROOT/'native',stage/'native',dirs_exist_ok=True)
 shutil.copytree(ROOT/'asmjit',stage/'asmjit',dirs_exist_ok=True)
 # Upstream CE omits x86/backend files. Restore only missing AsmJit files;
 # existing CE-vendored files always retain precedence.
 restored=[]
 for p in (a.asmjit/'src'/'asmjit').rglob('*'):
  if p.is_file():
   dst=stage/'asmjit'/p.relative_to(a.asmjit/'src'/'asmjit')
   if not dst.exists():dst.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,dst);restored.append(str(dst.relative_to(stage)))
 shutil.copytree(ROOT/'include',stage/'include',dirs_exist_ok=True)
 shutil.copytree(a.capstone/'include'/'capstone',stage/'include'/'capstone',dirs_exist_ok=True)
 obj=a.out/'objects';obj.mkdir(exist_ok=True);logs=a.out/'logs';logs.mkdir(exist_ok=True)
 env=os.environ.copy();env['ZIG_LOCAL_CACHE_DIR']=str(a.out/'zig-cache');env['ZIG_GLOBAL_CACHE_DIR']=str(a.out/'zig-global-cache')
 common=['-target','x86_64-windows-gnu','-O2','-fms-extensions','-Wno-ignored-pragmas','-Wno-invalid-source-encoding','-DWIN32_LEAN_AND_MEAN','-DASMJIT_STATIC','-DASMJIT_NO_AARCH64','-DNDEBUG','-I'+str(stage),'-I'+str(stage/'include'),'-I'+str(a.minhook/'include')]
 cpp=list(stage.glob('*.cpp'))+list((stage/'asmjit'/'core').glob('*.cpp'))+list((stage/'asmjit'/'x86').glob('*.cpp'))
 c=[a.minhook/'src'/x for x in ['buffer.c','hook.c','trampoline.c','hde/hde64.c']]
 cap=[a.capstone/x for x in ['cs.c','Mapping.c','MCInst.c','MCInstrDesc.c','MCRegisterInfo.c','SStream.c','utils.c']]+list((a.capstone/'arch'/'X86').glob('*.c'))
 sources=[(p,['c++','-std=c++20']) for p in cpp]+[(p,['cc','-std=c11']) for p in c]+[(p,['cc','-std=c99','-DCAPSTONE_HAS_X86','-DCAPSTONE_STATIC','-I'+str(a.capstone),' -I'+str(a.capstone/'include')]) for p in cap]
 if not a.skip_native:sources += [(stage/'native'/'AIDistrictScores.c',['cc','-std=c11','-DCSC_CE_INTEGRATION']),(stage/'native'/'hook_entry.S',['cc'])]
 header_digest=hashlib.sha256()
 for p in sorted(stage.rglob("*")):
  if p.is_file() and p.suffix in [".h",".inc"]:header_digest.update(str(p.relative_to(stage)).encode());header_digest.update(p.read_bytes())
 headers=header_digest.hexdigest()
 receipts=[]
 def compile(item):
  p,flags=item;flags=[x.strip() for x in flags];o=obj/(p.parent.name+'_'+p.name+'.o');cmd=[str(a.zig),*flags,*common,'-c',str(p),'-o',str(o)]
  stamp=hashlib.sha256((sha(p)+headers+json.dumps(cmd)).encode()).hexdigest();stampfile=o.with_suffix('.stamp')
  if o.exists() and stampfile.exists() and stampfile.read_text()==stamp:return dict(source=str(p),object=str(o),cached=True,sha256=sha(o))
  t=time.time();r=subprocess.run(cmd,cwd=stage,env=env,capture_output=True,text=True,errors='replace');(logs/(o.name+'.log')).write_text(r.stdout+r.stderr,encoding='utf-8');save(logs/(o.name+'.command.json'),cmd)
  if r.returncode:raise RuntimeError(str(p)+'\n'+r.stderr[:7000])
  stampfile.write_text(stamp);return dict(source=str(p),object=str(o),sha256=sha(o),elapsed=time.time()-t)
 with concurrent.futures.ThreadPoolExecutor(max_workers=a.jobs) as ex:
  for r in ex.map(compile,sources):receipts.append(r);print('compiled',Path(r['source']).name,flush=True)
 save(a.out/'compile-receipt.json',dict(sources=receipts,restored=restored,compiler_sha256=sha(a.zig),target_sha256=TARGET_SHA))
 if a.skip_native:return
 definition=a.out/'exports.def';lines=['LIBRARY GameCore_XP2_CE_FinalRelease','EXPORTS']
 for e in exports:
  name=e['name'];ordinal=e['ordinal'];assert name
  lines.append(f'  "{name}" @{ordinal}' if name in ['DllCreateGameContext','DllDestroyGameContext'] else f'  "{name}"="GameCore_XP2_FinalRelease.{name}" @{ordinal}')
 definition.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 dll=a.out/'GameCore_XP2_CE_FinalRelease.dll';cmd=[str(a.zig),'c++','-target','x86_64-windows-gnu','-shared','-o',str(dll),*[r['object'] for r in receipts],str(definition),'-lbcrypt','-luser32','-lkernel32','-ladvapi32','-lole32','-luuid']
 save(logs/'link-command.json',cmd);r=subprocess.run(cmd,cwd=stage,env=env,capture_output=True,text=True,errors='replace');(logs/'link.log').write_text(r.stdout+r.stderr,encoding='utf-8')
 if r.returncode:raise RuntimeError(r.stderr)
 assert all(sha(ROOT/p['path'])==p['sha256'] for p in pins),'Fork sources changed during build'
 built=PE(dll);actual=built.exports();assert [(e['name'],e['ordinal']) for e in actual]==[(e['name'],e['ordinal']) for e in exports]
 for e in actual:
  assert e['forwarder']==('GameCore_XP2_FinalRelease.'+e['name'] if e['name'] not in ['DllCreateGameContext','DllDestroyGameContext'] else None)
 save(a.out/'build-receipt.json',dict(dll=str(dll),sha256=sha(dll),bytes=dll.stat().st_size,exports=actual,imports=built.imports(),toolchain='Zig0.15.2 x86_64-windows-gnu; static libc++/libunwind; Win64 C boundary',native_runtime_tested=False,source_pins=pins))
 print('BUILT',dll,sha(dll))
if __name__=='__main__':main()
