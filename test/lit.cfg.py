import os
import lit.formats
import lit.TestingConfig

config: lit.TestingConfig.TestingConfig = config  # ty:ignore[unresolved-reference]

config.name = "llvm-ct"
config.test_format = lit.formats.ShTest()
config.suffixes = {".ll"}

config.substitutions.append(("%opt", "/usr/bin/opt"))
config.substitutions.append(("%FileCheck", "/usr/bin/FileCheck"))
config.substitutions.append(("%plugin", os.path.join(os.path.dirname(__file__), "..", "build", "CTPass.so")))
