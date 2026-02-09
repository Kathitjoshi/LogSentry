# Directory Guide

## What Each Folder Does (Simple Version)

### 1. dashboard/ - The Screen That Shows Everything

**In simple terms:** This is Netflix's home page - it shows you what's on

**Files inside:**
- `app.py` - The server that runs the website
- `static/js/dashboard.js` - The code that updates the numbers live  **(WE FIXED THIS)**
- `static/css/style.css` - Makes it look pretty
- `templates/index.html` - The actual webpage

**What happens when you visit http://localhost:5000:**
1. `app.py` runs
2. You see `index.html`
3. `dashboard.js` asks for data every 5 seconds
4. Data comes from `/api/logs`, `/api/anomalies`, `/api/stats`
5. Charts update automatically

---

### 2. log-processor/ - The Worker That Reviews Everything

**In simple terms:** This is a librarian who reads every book, catalogs it, and checks for damaged pages

**Folders inside:**
- `src/` - Main work area
  - `processor.py` - Pulls logs from RabbitMQ and processes them
  - `kvstore_client.py` - Saves logs to disk  **(WE FIXED THIS)**
  - `log_parser.py` - Understands log format
  
- `ml/` - The "smart detective" area
  - `anomaly_detector.py` - AI that says "this log looks suspicious!"
  - `feature_extractor.py` - Pulls out important info from logs

- `tests/` - Verification area
  - Test files to make sure things work

**Flow:**
```
RabbitMQ → processor.py reads → log_parser.py understands 
        → anomaly_detector.py checks → kvstore_client.py saves
```

---

### 3. log-producers/ - The Employees Sending Orders

**In simple terms:** These are waiters creating order tickets

**Files inside:**
- `sample_web_app.py` - Fake web server generating logs
- `sample_api_server.py` - Fake API generating logs
- `sample_database_app.py` - Fake database generating logs

**What they do:**
```
Create fake logs → Send to RabbitMQ queue every 0.5 seconds
```

These are used for **testing** - they generate demo data so we can see the whole system working

---

### 4. docker/ - Package Configuration

**In simple terms:** Recipe books for cooking

**Files inside:**
- `Dockerfile.processor` - How to package the processor
- `Dockerfile.dashboard` - How to package the dashboard
- `Dockerfile.producer` - How to package the producers

**What they do:**
```
Docker reads these files → Builds containers → Services run inside
```

---

### 5. config/ - The Settings

**Files inside:**
- `config.yaml` - Settings for all services

**Example settings:**
```yaml
rabbitmq:
  host: rabbitmq
  port: 5672
  username: admin
dashboard:
  host: 0.0.0.0
  port: 5000
```

---

### 6. scripts/ - Quick Commands

**Files inside:**
- `build.sh` - Build everything
- `start.sh` - Start everything
- `run_tests.sh` - Test everything

**Usage:**
```bash
bash scripts/build.sh
bash scripts/start.sh
```

---

### 7. logs-producers/ - More Fake Generators

Similar to `log-processors/` but generates different types of logs

---

### 8. data/ - Stuff That Gets Created When Running

**What's created:**
- `kvstore/` folder with processed logs stored as JSON files

**Automatically created:** Don't worry about this, it gets populated when processor runs

---

### 9. docs/ - Extra Documentation

Any additional guides and diagrams

---

## How Data Flows Through Folders

```
log-producers/
   ↓ (generates logs)
   
config/config.yaml
   ↓ (tells where to send)
   
Docker container: RabbitMQ
   ↓ (queue/mailbox)
   
log-processor/
   ├─ src/processor.py (pulls logs)
   ├─ src/log_parser.py (understands format)
   ├─ ml/anomaly_detector.py (checks if suspicious)
   └─ src/kvstore_client.py (saves to disk)
      ↓
   data/kvstore/ (stored as files)
      ↓
   
dashboard/
   ├─ app.py (web server)
   ├─ static/js/dashboard.js (reads from kvstore, updates charts)
   └─ templates/index.html (shows the page)
      ↓
   
Browser http://localhost:5000
   (you see the pretty dashboard!)
```

---

## Which Folder Does What?

| Folder | Creates | Reads | Stores |
|--------|---------|-------|--------|
| **log-producers/** | Fake logs | (nothing) | RabbitMQ |
| **log-processor/** | Processed logs | RabbitMQ | data/kvstore/ |
| **dashboard/** | Web pages | data/kvstore/ | (nothing) |
| **docker/** | Containers | (nothing) | (nothing) |
| **config/** | (nothing) | Used by all | (nothing) |
| **data/** | (nothing) | (nothing) | Processed logs |

---

## Quality Checklist

### Before Running:
- `docker-compose.yml` exists
- `config/config.yaml` exists
- `log-processor/src/processor.py` exists
- `dashboard/app.py` exists
- `log-producers/sample_web_app.py` exists

### After Running:
- `data/kvstore/` folder gets created
- `.json` files appear in `data/kvstore/`
- Dashboard shows increasing numbers

---

## Key Files & What They Do

| File | What It Does | If It Breaks |
|------|-------------|---|
| `docker-compose.yml` | Manages all containers | Nothing runs |
| `config/config.yaml` | Settings for all services | Services use defaults |
| `log-processor/src/processor.py` | Processes logs | Logs aren't processed |
| `dashboard/app.py` | Web server | Dashboard doesn't load |
| `dashboard/static/js/dashboard.js` | Updates dashboard | Dashboard doesn't update  **FIXED** |
| `log-processor/src/kvstore_client.py` | Saves logs to disk | Data isn't saved |

---

## Summary

Think of it like a **restaurant kitchen:**

```
Menu/Orders (log-producers)
    ↓
Order Queue (RabbitMQ via docker/config)
    ↓
Chef (log-processor/src/processor.py)
    ├─ Reads recipe (log_parser.py)
    ├─ Checks quality (ml/anomaly_detector.py)
    └─ Files receipt (kvstore_client.py → data/)
    ↓
Manager Display (dashboard/app.py + dashboard.js)
    ↓
Customer Screen & Numbers (Your browser)
```

Each folder has a specific job, and when they work together, you get a beautiful dashboard!
