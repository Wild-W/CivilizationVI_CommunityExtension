# Experimental district-site score API

This branch adds a generic native capability to Community Extension (CE). A mod
supplies numeric incentives for named player/city/district/plot pairs; the native
AI still evaluates alternatives, selects work and manages production. The API
contains no Quarter rules, adjacency formula, scripted construction or subsidies.
Those decisions belong in the consuming mod's Lua/SQL.

**Status:** experimental fork, not a released CE API. The complete portable CE
build preserves all157 original exports/ordinals and155 forwarders. After fixing
a long-string reporting failure exposed by the first run, the corrected build
completed a bounded Online100 test:73 positive and14,584 negative score changes,
all29,479 trace records reconstructed, and zero native faults. The consuming mod
computed policy in packaged Lua; Python only controlled/audited the test.

A second observation-only world1–2 in the same process reset policy to zero;
the exact saved6 reloaded with identical captured scanner facts. Native exit0,
exact preferences and unchanged package contents were verified. The corrected
string helper also passes36 isolated native checks. These results do not
establish AI quality, arbitrary save/unload behavior, general CE compatibility,
multiplayer, MSVC qualification or release readiness. See the
[qualification report](https://hennogous.github.io/Henno-Mods/ai/sources/project/docs/research/mods/csc-ce-policy-parity-20261104).

## Supported target and build

The native consumer is tied to the Windows x64 Gathering Storm original:

```
GameCore_XP2_FinalRelease.dll
SHA-256 324c51e9ea3531758842e16c69e6cddbefbb226c5b675c3d6e60111646c2e98c
PE timestamp 1719431003
Image size 12976128
```

Initialization verifies the original file hash, loaded image metadata and exact
function prefixes before installing this module's hooks. Required HKS exports
must resolve. Failure leaves this capability unavailable; consumers must check
availability and `ready` rather than assume that enabling a CE dependency is
enough. This fork does not extend support to another platform or GameCore build.

See [developer build instructions](../tools/README.md). The complete portable
compile/link route is verified; the updated MSVC C11/MASM project route remains
untested.
The compiler and Python are developer tools, not player runtime dependencies.

## Publish a complete numeric policy

All methods are gameplay Lua globals. Numeric return values are `1` for success
and `0` for rejection. Inputs are integers; validate them before invoking the
native functions. A consuming adapter should catch Lua errors and call Disable
on any rejected or partial publication.

| Global | Arguments | Purpose |
|---|---|---|
| `CE_AIDistrictScoresBegin` | `version, width, height, districtCount` | Disable current policy and start an empty staging batch. |
| `CE_AIDistrictScoresAllow` | `playerId` | Admit an original AI identity to the staging batch. |
| `CE_AIDistrictScoresEntry` | `playerId, cityId, districtIndex, plotIndex, deltaQ8` | Stage one adjustment. |
| `CE_AIDistrictScoresCommit` | none | Atomically publish the completed numeric table. |
| `CE_AIDistrictScoresDisable` | none | Retire active/staged policy and advance generation. |
| `CE_AIDistrictScoresStatus` | none | Return a JSON status string. |
| `CE_AIDistrictScoresTraceCount` | none | Return the retained native journal length. |
| `CE_AIDistrictScoresTraceRead` | `offset, limit, expectedGeneration` | Return a non-consuming JSON journal page. |

Call Begin, Allow for each eligible player, Entry for each distinct pair, then
Commit. A failed Allow or Entry invalidates the entire staging batch. Duplicate
pair keys are rejected. No partial batch becomes active. Begin itself disables
the previous policy even if its arguments are rejected. Commit requires a ready,
fault-free consumer; a fault requires context reset rather than another Commit.

`version` must be positive and greater than the last published version in this
context. Width and height are each 1â€“256. `districtCount` is 1â€“4096 and must cover
the engine's index range: use highest loaded district index plus one when indices
have gaps. Players are 0â€“63, cities are nonnegative, districts are 1 through
`districtCount-1`, and plot indices are zero-based `y * width + x`. A batch can
contain at most 4096 entries. A city ID is scoped to its player; it is not a
cross-player unique identity.

The caller owns original identity and world-state correctness. Capture human/AI
roles **before** autoplay changes them, admit only the intended original AI
players, and rebuild policy for each new context/load. The native allowlist does
not independently discover original humans. It also does not infer ownership,
legality, economic desirability or freshness from the caller's intent. Hook-side
identity, row and native eligibility checks are additional conditions, not a
replacement for a correct source snapshot.

## Score units and numerical guards

`deltaQ8` is a signed adjustment in the native fixed-point utility units:
**256 Q8 units = 1 utility unit**. It is neither an adjacency-yield amount nor an
instruction to build. The native output may subsequently receive other value
components or modifiers before competing with other work.

The current per-entry bounds are -262144 through +1048576 Q8 (-1024 through
+4096 utility), excluding zero. At most 4096 changed outputs are admitted per
published policy. These are diagnostic bounds, not recommended tuning weights.

The hook preserves the native returned output pointer. Zero and native-invalid
scores at or below -2559744 Q8 (-9999 utility) are not adjusted. Deductions do not
push results below -1048576 Q8 (-4096 utility), and do not further reduce a base
already at/below that floor. Computation uses an int64 intermediate and rejects
unsafe results, including values above `INT_MAX/2`, leaving headroom for later
native multipliers. A rejected value is retained unchanged and identified in
the journal. These guards deliberately preserve native invalidity rather than
turn an invalid candidate into a valid placement.

## Typed native hooks and scope

The mutation point is the ordinary district NetValue return/cache path. A Win64
assembly shim preserves its caller's row register and the original arguments.
The hook calls the original function, validates the exact admitted pair and
native eligibility, then conditionally adjusts the returned int32. It never
calls Lua or gameplay APIs from AI analysis jobs.

Three typed, read-only monitors observe completed district-cache calculation,
GetBestDistrict and PresentValue returns. They call the original routines and
record numeric data only. Cache and PresentValue records are restricted to
published pairs; GetBestDistrict observes the winner in a published city and can
therefore report another district type. This is not a complete all-candidate
trace, a request/queue log, or proof that an observed winner was constructed.

An SRW lock protects staging, published policy and journal data. A generation
captured before the original call prevents an older admitted invocation from
writing or appending after publication/reset. That does **not** make an entire
asynchronous city-analysis pass atomic, wait for every worker, refresh existing
cache entries or cancel retained requests. Previously cached values/requests can
continue to carry earlier influence after Disable.

## Read status and journal pages

Status includes ABI version, readiness, enabled state, generation, published
version, entry count, invocation/match/write/fault counters, the first 64 changed
samples, journal count/capacity and active native invocation count. `calls` is
hook activity, not the number of successful score changes. Each sample is:

```
[playerId, cityId, districtIndex, plotIndex, baseQ8, resultQ8]
```

The journal holds at most 16384 records per publication/context window. Exhaustion
disables incentives and increments faults rather than silently dropping evidence.
Its `k` values are `net`, `cache`, `best` and `pv`. Records also contain their
capture generation/version, player/city/district/plot, base/result, reason and
native eligibility. For net records, reason 0 means an applied adjustment,
2 a protected base, 3 a numerical rejection, and 4 write-budget exhaustion.
Read-only observations use reason 0 without implying a mutation. A `best` record
with district/plot -1 reports no winner.

TraceRead uses zero-based offsets and a limit of 1â€“64. Its JSON includes current
generation/version, total, offset, next, stale and records. A mismatched expected
generation returns `stale: true` with an empty record page. Invalid page arguments
return an error object. Pages do not consume the journal.

For a stable complete journal, Disable before reading, obtain the **new current**
generation from Status, and page with that same expected generation. Disable
retains records but changes generation; the records' capture generations may
therefore be older than the page generation. Old invocations cannot append after
retirement. Check active/fault status when the experiment requires worker
quiescence. Archive pages before beginning/committing the next policy because
Commit and context reset clear the journal. Reading while enabled can observe
additional appended records between pages; do not call that a frozen snapshot.

## Process and context lifecycle

CE's native hook initialization occurs once per process. Recreating hooks on
every new context could overwrite working trampoline pointers. Lua bindings are
registered again for each gameplay Lua state. Policy and diagnostic state reset
on context creation and before forwarding context destruction. This resets
admission/version state, not the process's already installed hooks. Invocation
`calls` remain cumulative; the active-invocation counter is not forcibly cleared.
Consumers
must reconstruct policy on load; the native numeric table is not serialized into
a save. Full lifecycle qualification remains a separate experimental gate.

## HKS JSON transport

The shipped HKS `hksi_lua_pushfstring` truncated long strings at 511 bytes in the
first native run. No free `lua_pushlstring`/`hksi_lua_pushlstring` export is present;
the LuaPlus member returning a LuaStackObject is not used as a guessed substitute.

The corrected helper pushes ASCII JSON in literal fragments of at most 240 bytes
through `%s`, checks space for two additional stack values, and joins fragments
incrementally with the verified public `hksi_lua_concat(lua_State*, int)` export.
It uses `luaL_checkstack(lua_State*, int, const char*)` and never treats JSON as a
format string. Only two temporary stack values are needed. The isolated mock
reproduces the formatter's 511-byte payload limit; long Status and 64-record
TraceRead serialization pass. The corrected full-game test also reconstructed
native status strings longer than1KB and complete trace pages of up to64records
through the actual HKS runtime. The consuming Lua logger frames large receipts
into bounded lines because the game's disk Lua.log independently truncates prints
at2047bytes; a full Tuner reply alone is not durable-log proof.
