# Project Explained Like You're 5

## What Does This Project Do?

Think of a restaurant:
- **Waiters** take customer orders → **Log Producers** (generate logs)
- **Waiters** put orders in a kitchen queue → **RabbitMQ** (message broker)
- **Chef** reads orders and cooks → **Log Processor** (processes logs)
- **Chef** notices if an order is weird → **Anomaly Detector** (ML detection)
- **Hostess** displays status on a screen → **Dashboard** (web interface)
- **Filing cabinet** stores receipts → **KVStore** (data storage)

---

## Folder Breakdown

### **dashboard/** - The Restaurant's Menu Board
**What it does:** Shows customers what's happening
- `app.py` - Flask web server (the menu board itself)
- `static/js/dashboard.js` - Page updates live (shows log counts, anomalies)
- `static/css/style.css` - Makes it look pretty
- `templates/index.html` - The actual webpage

**In simple terms:** This is what you see at http://localhost:5000 - a real-time display of all logs and alerts


### **log-processor/** - The Chef's Station
**What it does:** Reads orders (logs) from the queue and processes them
- `src/processor.py` - Main processor that pulls logs from RabbitMQ
- `src/kvstore_client.py` - Saves processed logs to storage
- `src/log_parser.py` - Reads log format
- `ml/anomaly_detector.py` - The "weird detector" (uses AI to find problems)
- `ml/feature_extractor.py` - Extracts important info from logs

**In simple terms:** This is the worker that reads every log, checks if it's normal or suspicious, and saves it


### **log-producers/** - The Waiters
**What it does:** Creates fake logs and sends them to RabbitMQ
- `sample_web_app.py` - Simulates a web server sending logs
- `sample_api_server.py` - Simulates an API sending logs
- `sample_database_app.py` - Simulates a database sending logs

**In simple terms:** These generate test data (like a practice drill) so we can see if the dashboard works


### **rabbitmq/** (Via Docker)
**What it does:** Acts as a smart mailbox
- Producers PUT logs in → Processor TAKES logs out
- Guarantees delivery (logs won't get lost even if processor crashes)
- Multiple consumers can read from it

**In simple terms:** The reliable delivery system - like a post office


### **docker/** - Deployment Configuration
**What it does:** Tells Docker how to package each service
- `Dockerfile.processor` - How to run log processor in a container
- `Dockerfile.dashboard` - How to run dashboard in a container
- `Dockerfile.producer` - How to run producers in a container

**In simple terms:** Recipes for Docker to build portable environments


### **config/** - Settings
- `config.yaml` - Configuration for all services (RabbitMQ credentials, ports, etc.)

**In simple terms:** The instruction manual for how services should work


### **scripts/** - Helper Tools
- `build.sh` - Build everything
- `run_tests.sh` - Test everything works
- `start.sh` - Start everything

**In simple terms:** Shortcuts to do common tasks


---

## How Data Flows (The Journey of a Log)

```
┌──────────────┐
│   Waiters    │ (Log Producers)
│  Generate   │------->    ┌─────────────┐
│   Orders    │            │  RabbitMQ   │
└──────────────┘            │    Queue   │
                            └─────────────┘
                                   │
                                   │ (Messages)
                                   ▼
                            ┌──────────────────┐
                            │  Log Processor   │
                            │  Reads & Checks  │
                            │  (Normal or Bad) │
                            └──────────────────┘
                                   │
                                   ├─────> ┌─────────────┐
                                   │       │  KVStore    │
                                   │       │  (Storage)  │
                                   │       └─────────────┘
                                   │
                                   └─────> ┌──────────────┐
                                           │  Dashboard   │
                                           │  (Displays)  │
                                           └──────────────┘
```

---

## What Happens When You Run `docker-compose up --build`

```
1. RabbitMQ wakes up
   ↓
2. Dashboard starts listening on port 5000
   ↓
3. Log Producer creates fake logs
   ↓
4. Logs go into RabbitMQ queue
   ↓
5. Log Processor pulls logs from queue
   ↓
6. Processor saves to KVStore (/app/data)
   ↓
7. Dashboard reads from KVStore
   ↓
8. Open http://localhost:5000 and see live updates!
```

---

## Dashboard Metrics (What You See)

| Metric | Meaning |
|--------|---------|
| **TOTAL LOGS** | How many logs have been processed |
| **ANOMALIES** | How many suspicious logs were found |
| **CACHE HIT RATE** | Percentage of logs that are anomalies |
| **STORAGE SIZE** | How much disk space logs use |

---

## Technologies Used

| Tool | What It Does |
|------|-------------|
| **Python** | Programming language for everything |
| **Flask** | Web framework for dashboard |
| **RabbitMQ** | Message broker (reliable queue) |
| **Docker** | Container system (portable packages) |
| **Isolation Forest** | AI algorithm to detect anomalies |
| **scikit-learn** | ML library for anomalies |

---

## Current Status (After Fixes)

- Dashboard shows **real log counts**
- Dashboard shows **real anomaly counts**
- Dashboard shows **real storage size**
- 280+ logs successfully processed and displayed
- All three services communicate perfectly
- Data persists between requests

---

## What Was Broken (Before Fixes)

| Issue | Cause | Fix |
|-------|-------|-----|
| Dashboard showed 0 logs | JavaScript had wrong field name | Changed `total_keys` → `num_keys` |
| Dashboard showed NaN% | JavaScript tried to divide by non-existent fields | Calculate from actual data |
| Storage showed 0.00 MB | Dashboard couldn't read processor's files | Shared Docker volume between containers |

