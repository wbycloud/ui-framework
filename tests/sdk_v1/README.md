# Frozen SDK v0.1.0 compatibility fixtures

These five public headers are copied byte-for-byte from Git tag v0.1.0
(commit b89438ab8127c419fde152b5e794bbe509a45de4). They compile the EDA,
lifecycle and C++ counter fixtures as real API1/ABI1 consumers of the new DLL.
Do not update these snapshots when the current SDK changes. Current applications
should use the public include/ directory. Original test source assertions remain
unchanged; obsolete API2-rejection/class-rejection assertions are reconciled
separately in the validation report.
