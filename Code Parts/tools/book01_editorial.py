"""Reader-edition titles; original Markdown headings remain byte-for-byte intact.

The reader currently assembles Chapters 01–16 of a 39-chapter foundation
manuscript. These are editorial titles, not assertions of complete ISO C++23
coverage or a rewrite of the original chapter content.
"""
SERIES_TITLE = "From Bits to Research"
SERIES_SUBTITLE = "Computing, Mathematics, Graphics, Simulation, and Scientific Discovery"
SERIES_POSITION = "Book One of Seven"

BOOK_TITLE = "From Bits to Parallel Systems"
BOOK_SUBTITLE = "C++23, CPU Execution, GPU Memory, and Reliable Software"
PRINT_VOLUMES = (
    (1, 8, "Volume I — From Bits to Programs",
     "Representation, Ownership, and Reliable C++23"),
    (9, 16, "Volume II — From Programs to GPUs",
     "Memory Models, Parallel Execution, Vulkan, and Metal"),
)
CHAPTER_TITLES = (
    "Information, Binary Representation, and Running Programs",
    "Memory, Object Representation, and Lifetimes",
    "Value Semantics, Constructors, and Resource Ownership",
    "Testing C++ Behavior and Failure Guarantees",
    "Generic Programming and Typed Data Pipelines",
    "Identifiers, Registries, and Resource Management",
    "Interfaces, Data Structures, and CPU Execution",
    "Algorithms, Dependency Graphs, and Parallel Work",
    "GPU Architecture, Memory Access, and Ordering",
    "Build Systems, Profiling, and Reproducible Evidence",
    "Const Correctness, Borrowing, and Buffer Lifetimes",
    "Asynchronous Commands and Synchronization Contracts",
    "Vulkan Devices, Memory, and Resource Descriptors",
    "Vulkan Submission, Frame Lifetimes, and Synchronization",
    "Metal Command Execution and Frame Measurement",
    "GPU Performance Analysis and Platform Ownership",
)
assert len(CHAPTER_TITLES) == 16
