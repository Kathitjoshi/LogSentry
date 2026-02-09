# Project Structure

Current directory structure of the Distributed Log Analyzer project (after cleanup).

```
distributed-log-analyzer-FINAL/
│
├── Documentation Files
│   ├── README.md                       # Main project documentation
│   ├── PROJECT_EXPLANATION.md          # ELI5 guide to the project
│   ├── QUICKSTART.md                   # Quick start guide
│   ├── SETUP_GUIDE.md                  # Detailed setup instructions
│   ├── LOCAL_SETUP.md                  # Local development setup
│   ├── PROJECT_STRUCTURE.md            # This file
│   ├── FILE_INVENTORY.md               # Detailed file inventory
│   ├── CHANGES.md                      # Recent changes
│   ├── LICENSE                         # MIT License
│   └── Makefile                        # Build automation
│
├── Configuration & Setup
│   ├── docker-compose.yml              # Docker container orchestration
│   ├── requirements.txt                # Python dependencies
│   ├── setup.sh                        # Setup script
│   ├── .gitignore                      # Git ignore rules
│   └── config/
│       └── config.yaml                 # Service configuration (RabbitMQ, ports, etc.)
│
├── DASHBOARD (Web UI)
│   ├── app.py                          # Flask web server
│   ├── __init__.py                     # Python package marker
│   ├── templates/
│   │   └── index.html                  # Main webpage
│   ├── api/
│   │   └── __init__.py                 # API package
│   └── static/                         # Assets (CSS, JS)
│       ├── css/
│       │   └── style.css               # Dashboard styling
│       └── js/
│           └── dashboard.js            # Live charts & updates 
│
├── LOG PROCESSOR (Core Worker)
│   ├── __init__.py                     # Python package marker
│   ├── src/                            # Main processing code
│   │   ├── __init__.py
│   │   ├── processor.py                # Main processor (reads RabbitMQ, processes logs)
│   │   ├── kvstore_client.py           # File-based KV store client 
│   │   ├── log_parser.py               # Parses log formats
│   │   └── __pycache__/                # Compiled Python
│   ├── ml/                             # Machine Learning components
│   │   ├── __init__.py
│   │   ├── anomaly_detector.py         # Isolation Forest ML model
│   │   ├── feature_extractor.py        # Extracts features from logs
│   │   └── __pycache__/
│   └── tests/                          # Unit tests
│       ├── __init__.py
│       ├── test_processor.py           # Processor tests
│       ├── test_anomaly_detector.py    # ML tests
│       └── __init__.py
│
├── LOG PRODUCERS (Test Data Generators)
│   ├── __init__.py                     # Python package marker
│   ├── sample_web_app.py               # Generates web server logs
│   ├── sample_api_server.py            # Generates API logs
│   └── sample_database_app.py          # Generates database logs
│
├── DOCKER (Container Configuration)
│   ├── Dockerfile.processor            # Build image for log processor
│   ├── Dockerfile.dashboard            # Build image for dashboard
│   ├── Dockerfile.producer             # Build image for log producers
│   ├── Dockerfile.alerting             # (Unused in current setup)
│   ├── Dockerfile.kvstore              # (Unused in current setup)
│   └── Dockerfile.consumer             # (Unused in current setup)
│
├── KVSTORE (Optional - Custom Storage)
│   ├── CMakeLists.txt                  # C++ build config
│   ├── include/                        # C++ headers
│   │   ├── kvstore.h
│   │   ├── lsm_engine.h
│   │   ├── memtable.h
│   │   ├── sstable.h
│   │   ├── wal.h
│   │   ├── bloom_filter.h
│   │   ├── lru_cache.h
│   │   └── compaction.h
│   ├── src/                            # C++ implementation
│   │   ├── kvstore.cpp
│   │   ├── lsm_engine.cpp
│   │   ├── memtable.cpp
│   │   ├── sstable.cpp
│   │   ├── wal.cpp
│   │   ├── bloom_filter.cpp
│   │   ├── lru_cache.cpp
│   │   ├── compaction.cpp
│   │   └── server.cpp
│   ├── python_bindings/
│   │   └── kvstore_binding.cpp         # Python wrapper for C++ code
│   └── tests/                          # C++ unit tests
│       ├── test_kvstore.cpp
│       ├── test_lsm_engine.cpp
│       ├── test_memtable.cpp
│       ├── test_sstable.cpp
│       ├── test_wal.cpp
│       └── test_bloom_filter.cpp
│
├── ALERTING (Optional Email Alerts)
│   ├── __init__.py
│   ├── alert_manager.py                # Manages alerts
│   └── notifications.py                # Send email/Slack notifications
│
├──  SCRIPTS (Helper Tools)
│   ├── build.sh                        # Build all components
│   ├── run_tests.sh                    # Run test suite
│   ├── setup.sh                        # Initial setup
│   ├── start.sh                        # Start all services
│   ├── benchmark_kvstore.py            # Performance testing
│
├── DOCS (Documentation)
│   └── (Architecture diagrams, guides, etc.)
│
├──  GIT CI/CD
│   └── .github/
│       └── workflows/
│           └── ci.yml                  # GitHub Actions pipeline
│
├──  DATA (Runtime Files - Generated)
│   └── kvstore/                        # Processed logs storage
│       └── (*.json files created by processor)
│
└──  SHARED VOLUME (Docker)
    └── processor_data:/app/data        # Shared between processor & dashboard 
```

---

##  Key Files & Their Functions

### Critical Files (System Won't Work Without These)

| File | Purpose |
|------|---------|
| `docker-compose.yml` | Orchestrates all containers |
| `log-processor/src/processor.py` | Reads logs and processes them |
| `dashboard/app.py` | Web server for live display |
| `log-producers/sample_web_app.py` | Generates test logs |
| `config/config.yaml` | Configuration settings |

### Important Files (Fixed in Previous Steps)

| File | Issue | Fix |
|------|-------|-----|
| `dashboard/static/js/dashboard.js` | Showed 0 logs/NaN% |  Fixed field references |
| `docker-compose.yml` | Dashboard couldn't read processor data |  Added shared volume |

### Optional Files (Advanced Features)

| File | Purpose |
|------|---------|
| `kvstore/src/` | Custom C++ key-value store (not used currently) |
| `alerting/` | Send email alerts (not enabled) |

---
│   ├── consumer/                   # RabbitMQ consumers
│   │   ├── __init__.py
│   │   ├── log_consumer.py        # Main log consumer
│   │   └── parser.py              # Multi-format log parser
│   │
│   ├── producer/                   # Log producers
│   │   ├── __init__.py
│   │   └── sample_app.py          # Sample log generator
│   │
│   ├── ml/                         # Machine Learning
│   │   ├── __init__.py
│   │   ├── anomaly_detector.py    # Isolation Forest anomaly detection
│   │   └── trainer.py             # Model training script
│   │
│   └── tests/                      # pytest test suite
│       ├── __init__.py
│       └── test_processor.py      # Unit and integration tests
│
├── dashboard/                       # Flask web dashboard
│   ├── __init__.py
│   ├── app.py                      # Main Flask application
│   │
│   ├── templates/                  # HTML templates
│   │   └── index.html             # Main dashboard page
│   │
│   ├── static/                     # Static assets
│   │   ├── css/
│   │   │   └── style.css          # Dashboard styles
│   │   └── js/
│   │       └── dashboard.js       # Dashboard JavaScript
│   │
│   └── api/                        # API routes (placeholder)
│       └── __init__.py
│
├── docker/                          # Docker configurations
│   ├── Dockerfile.consumer         # Consumer container
│   ├── Dockerfile.dashboard        # Dashboard container
│   └── Dockerfile.producer         # Producer container
│
├── scripts/                         # Utility scripts
│   └── benchmark_kvstore.py        # Performance benchmarking
│
├── config/                          # Configuration files (empty, for future use)
│
└── data/                            # Data directory (created at runtime)
    └── logs.db/                    # KV store database files
```

## File Count by Type

- **C++ Files**: 10 headers + 5 implementations + 4 test files = 19 files
- **Python Files**: 10 modules + 1 test file = 11 files
- **Web Files**: 1 HTML + 1 CSS + 1 JS = 3 files
- **Docker Files**: 3 Dockerfiles + 1 docker-compose.yml = 4 files
- **Config Files**: 1 CMakeLists.txt + 1 requirements.txt + 1 Makefile = 3 files
- **CI/CD**: 1 GitHub Actions workflow = 1 file
- **Documentation**: 1 README + 1 QUICKSTART + 1 LICENSE = 3 files
- **Other**: 1 .gitignore + 1 setup.sh + __init__.py files = 10+ files

**Total: 45+ files**

## Key Components

### 1. Custom KV Store (C++)
- **MemTable**: Skip list for fast in-memory operations
- **WAL**: Write-Ahead Log for crash recovery
- **SSTable**: Immutable sorted tables on disk
- **Bloom Filter**: Reduces unnecessary disk reads
- **LSM Engine**: Coordinates all components, handles compaction

### 2. Log Processing (Python)
- **Consumer**: Reads from RabbitMQ, stores in KV store
- **Parser**: Supports multiple log formats (JSON, syslog, Apache, Nginx)
- **ML Detector**: Isolation Forest for anomaly detection
- **Producer**: Generates sample logs for testing

### 3. Web Dashboard (Flask)
- **Real-time stats**: Total logs, anomalies, services
- **Charts**: Timeline and distribution visualizations
- **Log viewer**: Recent logs and anomalies
- **Service monitoring**: Status of all services

### 4. DevOps
- **Docker Compose**: Full stack orchestration
- **GitHub Actions**: CI/CD with automated testing
- **Makefile**: Convenient build and test commands

## Data Flow

```
1. Log Producer → RabbitMQ
2. RabbitMQ → Log Consumer
3. Log Consumer → Parser → ML Detector
4. Log Consumer → KV Store (C++)
5. KV Store → Dashboard
6. Dashboard → User Browser
```

## Technology Stack

| Component | Technology |
|-----------|-----------|
| Storage Engine | C++17, LSM-tree |
| Message Queue | RabbitMQ |
| ML/AI | scikit-learn, Isolation Forest |
| Backend | Python 3.10+, Flask |
| Frontend | HTML5, Bootstrap 5, Chart.js |
| Testing | Google Test (C++), pytest (Python) |
| CI/CD | GitHub Actions |
| Containerization | Docker, Docker Compose |
| Build System | CMake, Make |

## Build Artifacts

After building, you'll find:
- `kvstore/build/` - Compiled C++ binaries
- `kvstore/build/libkvstore_lib.a` - Static library
- `kvstore.so` - Python extension module
- `data/logs.db/` - Database files (SSTables, WAL)
- `htmlcov/` - Test coverage reports

## Important Notes

1. **Database Location**: Default is `/data/logs.db` in containers, configurable via env vars
2. **Test Files**: Some test files are stubs; main tests are consolidated in `test_wal.cpp`
3. **Python Bindings**: Require pybind11, built via CMake
4. **Docker Networks**: All containers communicate via `log-network` bridge
5. **Volumes**: Persistent data stored in `log-data` volume

## Getting Started

1. **Quick Start**: `docker-compose up`
2. **Development**: `./setup.sh` then start services manually
3. **Testing**: `make test`
4. **Benchmarking**: `make benchmark`

See QUICKSTART.md for detailed instructions.
