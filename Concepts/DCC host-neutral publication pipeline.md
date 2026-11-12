# DCC host-neutral publication pipeline

Blender, Maya, Houdini and Nuke expose different scene and graph models. A production engine should not let those host APIs leak into its core publication logic.

Use adapters:

host API → host-neutral record → validation → deterministic build plan → conversion/optimization → staging → verification → atomic publication.

The neutral record must preserve identity, units/coordinate conventions, schema version, normalized relative paths, dependency information and content/build identity. Paths are locations, not durable object identities.

Blender requires an authored-versus-evaluated-state policy. Maya requires DAG/reference/instance identity policy. Houdini requires recipe-versus-cooked-output and time-sample policy. Nuke requires frame-range, channel, color and premultiplication contracts.

The host-neutral layer can be tested in ordinary CI; host adapters need smaller in-application integration tests.

**Executable witness:** `tools/labs/phase30_pipeline_contract_lab.cpp`.

**Next depth:** real Blender/Maya/Houdini/Nuke fixtures, symlink-safe filesystem publication, SHA-256 content addressing and failure-injection recovery.
