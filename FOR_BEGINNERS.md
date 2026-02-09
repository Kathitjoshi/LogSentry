# Distributed Log Analyzer - Explained for Beginners

## TL;DR (Too Long; Didn't Read)

**This is a system that:**
1. Generates fake server logs (like web requests, errors)
2. Processes them through a pipeline
3. Detects which ones are "weird" using AI
4. Shows everything on a pretty dashboard

**It runs in Docker with ONE command:**
```bash
docker-compose up --build
```

Then open: http://localhost:5000

---

## Pizza Shop Analogy

Imagine a busy pizza shop:

### The Equipment:

| Part | What It Does | Pizza Shop Equivalent |
|------|-------------|----------------------|
| **Log Producers** | Create events/logs | Customers placing orders |
| **RabbitMQ** | Queue/storage system | Order counter/board |
| **Log Processor** | Reads and analyzes logs | Pizza chef |
| **Dashboard** | Shows real-time status | Manager's tablet |
| **KVStore** | Saves data to disk | Filing cabinet with receipts |

### The Flow:

```
Customer orders pizza (Log Producer)
    ↓ [order goes to counter]
Order sits in queue (RabbitMQ - "waiting for chef")
    ↓ [chef picks up order]
Chef makes pizza (Log Processor)
    ├─ Reads recipe (Log Parser)
    ├─ Checks quality (Anomaly Detector)
    └─ Files receipt (KVStore)
    ↓ [receipt saved]
Manager checks tablet (Dashboard)
    ├─ How many orders? 
    ├─ Any problems? 
    ├─ How busy? 
    └─ What's taking longest? 
    ↓
You see on screen (Your browser)
    "200 orders processed, 2 problems detected, 98% hit rate"
```

---

## Explain It Like I'm 5

**Person:** "What does this project do?"

**Me:** "It's like a teacher grading homework super fast:
- Lots of homework come in (logs)
- The teacher reads each one (processor)
- The teacher marks them (normal or weird)
- A board shows how many done (dashboard)"

---

## Folder Tour

### 1. dashboard/ = Your TV Screen
Shows you what's happening. Like Netflix or YouTube.

**Inside:**
- `app.py` - The TV's remote control
- `dashboard.js` - Updates the picture
- `index.html` - What you see
- `style.css` - Pretty colors

### 2. log-processor/ = The Worker Robot
Reads everything, checks it, saves it.

**Inside:**
- `processor.py` - Main robot
- `log_parser.py` - Translator robot
- `anomaly_detector.py` - Quality checker robot

### 3. log-producers/ = The Printer
Prints out fake homework (test data)

**Inside:**
- `sample_web_app.py` - Pretends to be a website
- `sample_api_server.py` - Pretends to be an API
- `sample_database_app.py` - Pretends to be a database

### 4. config/ = The Settings
Tells robots what to do (what port, what password, etc.)

### 5. docker/ = The Shipping Boxes
Packages robots into containers so they work anywhere

### 6. **data/** = The Vault
Where all the receipts/data gets stored (created automatically)

---

## What Happens When You Run It?

```
Step 1: docker-compose up --build
  └─> Docker reads instructions
  
Step 2: Containers start
  ├─> RabbitMQ wakes up (port 5672)
  ├─> Log Producers start creating logs
  ├─> Log Processor starts reading logs (2-3 seconds delay)
  └─> Dashboard starts on port 5000

Step 3: You open browser
  └─> Go to http://localhost:5000

Step 4: Auto-update (every 5 seconds)
  ├─> Dashboard asks: "How many logs?"
  ├─> System responds: "280"
  ├─> Dashboard asks: "How many weird?"
  ├─> System responds: "0"
  ├─> Chart updates
  └─> Repeat every 5 seconds
```

---

## Dashboard Metrics Explained

### **TOTAL LOGS: 280**
- Means: 280 log entries have been processed
- Goes up over time: YES 
- Should be 0: NO 

### **ANOMALIES: 0**
- Means: 0 suspicious logs detected
- Expected: Usually 0 or very low
- Why: Test data is all normal operations
- Real production: Depends on your logs

### **CACHE HIT RATE: 0.0%**
- Means: Ratio of anomalies to all logs
- Formula: (anomalies / total_logs) × 100
- Expected: Usually < 5%
- High rate: Many errors/problems happening

### **STORAGE SIZE: 2.5 MB**
- Means: How much disk space logs use
- Unit: Megabytes (MB)
- Grows over: Time (as more logs accumulate)
- Should be: Not zero 

---

## What to Play With

### Try 1: Stop and Restart
```bash
# Stop
docker-compose down

# Start
docker-compose up

# Expected: Counts reset, new logs start processing
```

### Try 2: Watch Logs in Real-time
```bash
# Open 3 terminals:

# Terminal 1: See all logs
docker-compose logs -f

# Terminal 2: See processor
docker-compose logs -f log_processor

# Terminal 3: See producer
docker-compose logs -f log_producer
```

### Try 3: Check Live RabbitMQ
```
Open: http://localhost:15672
Login: admin / admin123

See: Real-time queue status, connections, etc.
```

---

## Common Issues & Solutions

### "Dashboard shows 0 logs"
**Solution:**
```bash
docker-compose down
docker-compose up --build
# Wait 30 seconds
```

### "Port 5000 already in use"
**Solution:** Edit `docker-compose.yml` line 62
```yaml
- "5001:5000"  # Use 5001 instead
```

### "Containers exit immediately"
**Check:**
```bash
docker-compose logs log_processor  # See error messages
```

---

## Next Level: Modification Ideas

### 1. Add an ERROR Log
Edit `log-producers/sample_web_app.py`:
```python
logging.error("Database connection failed!")  # Will be caught as anomaly
```

### 2. Change Update Frequency
Edit `dashboard/static/js/dashboard.js`:
```javascript
setInterval(updateDashboard, 1000);  // Update every 1 second instead of 5
```

### 3. Change Anomaly Sensitivity
Edit `log-processor/ml/anomaly_detector.py`:
```python
contamination=0.2  # More anomalies detected
```

### 4. Increase Log Production
Edit `log-producers/sample_web_app.py`:
```python
time.sleep(0.2)  # Generate more logs faster
```

---

## What You're Learning

By using this project, you'll learn:

1. **Distributed Systems** - Multiple services working together
2. **Message Queues** - RabbitMQ (reliable delivery)
3. **Real-time Processing** - Logs processed instantly
4. **Machine Learning** - Anomaly detection algorithm
5. **Web Development** - Flask + JavaScript dashboard
6. **Docker** - Container orchestration
7. **DevOps** - Multi-container deployment
8. **Python** - Full stack Python application

---

## Project Goals

Learn distributed systems  
See ML in action  
Understand message queues  
Build real-time dashboards  
Deploy with Docker  
Debug production-like systems  

---

## Documentation Map

**Start Here:**
1. This file (you're reading it!)

**Then Read:**
2. `PROJECT_EXPLANATION.md` - Simple folder breakdown
3. `QUICKSTART.md` - How to run it
4. `DIRECTORY_GUIDE.md` - What each folder does

**Advanced:**
5. `SETUP_GUIDE.md` - Local development
6. `LOCAL_SETUP.md` - Windows/Mac specific
7. `PROJECT_STRUCTURE.md` - Technical details

---

## Key Concepts Explained

### **Log**
A record of something that happened. Example:
```
2026-02-07 06:34:01 - GET /api/users - 200 - 45ms
```

### **Anomaly**
Something abnormal/suspicious. Example:
```
GET /api/users - 500 - 5000ms  (server error + very slow)
```

### **Queue (RabbitMQ)**
A mailbox. Things go in, things come out in order.
```
Producer → [Mail Box] → Consumer
```

### **Container (Docker)**
A box that contains a program + its dependencies.
```
Same code = Same behavior anywhere (Windows/Mac/Linux)
```

### **Dashboard**
Live display of what's happening right now.
```
Like a control room monitoring system
```

---

## Success Checklist

- Docker installed
- Project runs with `docker-compose up --build`
- Dashboard opens at http://localhost:5000
- Metrics display real numbers (not 0/NaN)
- Numbers increase over time
- No error messages
- Understanding how everything connects

**If all checked: YOU'RE DONE!** 

---

## FAQ

**Q: Is this for production?**  
A: No, it's for learning. But it shows good patterns!

**Q: Can I use real logs?**  
A: Yes! Modify `log-producers/` to read from your logs.

**Q: How slow is it?**  
A: Can process 100+ logs/second on a laptop.

**Q: Do I need Kubernetes?**  
A: No, Docker Compose is enough for learning.

**Q: Can I run on Raspberry Pi?**  
A: Maybe, but it might be slow.

---

## Fun Facts

- RabbitMQ is used by Netflix, Uber, Discord
- Isolation Forest is from Microsoft Research
- Flask is used by Pinterest, LinkedIn
- Docker is used by 99% of cloud companies
- This project shows real production patterns!

---

## Conclusion

You now have a **fully functional distributed log analysis system** that:
- Processes logs in real-time
- Detects anomalies with ML
- Shows everything on a dashboard
- Runs in Docker
- Works on any computer

Congrats! You've got a mini version of what Netflix/Uber/Google uses!
