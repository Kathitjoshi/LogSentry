# Quick Start Guide

Get the Distributed Log Analyzer running in **2 minutes** with Docker!

---

## The Easiest Way (Docker - Recommended)

### Step 1: Clone/Navigate to Project
```bash
cd LogSentry
```

### Step 2: Start Everything (ONE COMMAND)
```bash
docker-compose up --build
```

### Step 3: Open Dashboard
Open your browser → **http://localhost:5000**

**That's it!** You'll see:
- Live log count increasing (every log is stored)
- Anomalies detected
- Storage size growing
- Real-time charts updating

---

## What You'll See

```
┌─────────────────────────────────────┐
│   Distributed Log Analyzer          │
│  Real-time log monitoring with ML   │
├─────────────────────────────────────┤
│ TOTAL LOGS: 280   ANOMALIES: 0      │
│ CACHE HIT RATE: 0.0%  STORAGE: 2.5MB│
├─────────────────────────────────────┤
│ [Chart] Log Levels Over Time        │
│ [Chart] Anomaly Detection Timeline  │
├─────────────────────────────────────┤
│ [Tables] Recent Logs                │
│ [Tables] Recent Anomalies           │
└─────────────────────────────────────┘
```

---

## Monitor in Real-Time

```bash
# Watch all container logs
docker-compose logs -f

# Watch specific service
docker-compose logs -f log_processor
docker-compose logs -f log_dashboard
docker-compose logs -f log_producer
```

---

## Control the Containers

```bash
# Stop everything
docker-compose down

# Start again
docker-compose up

# Rebuild on changes
docker-compose up --build --no-deps log_processor

# Remove all volumes (clear data)
docker-compose down -v
```

---

## Access Control Panels

| Service | URL | Purpose |
|---------|-----|---------|
| **Dashboard** | http://localhost:5000 | View logs & anomalies |
| **RabbitMQ** | http://localhost:15672 | Message broker UI |

**RabbitMQ Login:**
- Username: `admin`
- Password: `admin123`

---

## Troubleshooting

### Dashboard shows 0 logs?
```bash
# Restart everything
docker-compose down
docker-compose up --build
# Wait 10 seconds for startup
```

### Port already in use?
```bash
# Change port in docker-compose.yml
dashboard:
  ports:
    - "5001:5000"  # Use 5001 instead
```

### Want to see raw logs being processed?
```bash
# Terminal 1: See producer
docker-compose logs -f log_producer

# Terminal 2: See processor
docker-compose logs -f log_processor

# Terminal 3: See dashboard requests
docker-compose logs -f log_dashboard
```

---

## Cleanup

```bash
# Remove containers & networks (keeps volumes)
docker-compose down

# Remove everything including data
docker-compose down -v

# Remove images
docker-compose down --rmi all
```

---

## Sample Log Output

You'll see logs like:

```
log_producer   | [web-server] INFO: GET /api/users - 200 - 45ms
log_processor  | 2026-02-07 06:33:26 - __main__ - INFO -  Stored log #109
log_dashboard  | 172.18.0.1 - - [07/Feb/2026 06:33:29] "GET /api/logs HTTP/1.1" 200
```

This means:
- **log_producer**: Generated a fake log
- **log_processor**: Successfully stored it
- **log_dashboard**: Serve the data to your browser

---

## Next Steps

1. **Understand the Architecture** → Read `PROJECT_EXPLANATION.md`
2. **See All Files** → Read `PROJECT_STRUCTURE.md`
3. **Local Development** → Read `LOCAL_SETUP.md`
4. **Advanced Setup** → Read `SETUP_GUIDE.md`

---

## Verification Checklist

After `docker-compose up --build`:

- [ ] Dashboard loads at http://localhost:5000
- [ ] You see metrics (Total Logs > 0)
- [ ] Logs are increasing every 5 seconds
- [ ] Storage size is growing
- [ ] No error messages in terminal

**If something's wrong:**
```bash
# Check service health
docker-compose ps

# Should show all services as "Up"
```

---

## Success!

**If you can see the dashboard with increasing log counts, **you're done!**

The system is:
- Generating fake logs
- Processing them through the queue
- Detecting anomalies
- Displaying results live
sudo apt-get install -y libgtest-dev
```

### Step 2: Run Setup Script

```bash
chmod +x setup.sh
./setup.sh
```

### Step 3: Start RabbitMQ

```bash
docker run -d \
    --name rabbitmq \
    -p 5672:5672 \
    -p 15672:15672 \
    rabbitmq:3-management
```

### Step 4: Start Services

**Terminal 1 - Log Consumer:**
```bash
python3 log_processor/consumer/log_consumer.py
```

**Terminal 2 - Dashboard:**
```bash
python3 dashboard/app.py
```

**Terminal 3 - Sample Log Producer:**
```bash
python3 log_processor/producer/sample_app.py --rate 10
```

## Verify Installation

1. **Check Dashboard**: Open http://localhost:5000
   - You should see statistics cards
   - Timeline chart should be updating
   - Recent logs should appear

2. **Check RabbitMQ**: Open http://localhost:15672
   - Login: guest / guest
   - Check "Queues" tab for "logs" queue
   - Should see messages being processed

3. **Check KV Store**: Run benchmark
   ```bash
   python3 scripts/benchmark_kvstore.py --num-ops 10000
   ```

## Common Commands

### Using Makefile

```bash
make help          # Show all available commands
make build         # Build C++ components
make test          # Run all tests
make benchmark     # Run performance tests
make run           # Start with Docker Compose
make stop          # Stop services
make clean         # Clean build artifacts
```

### Docker Commands

```bash
# Start services
docker-compose up -d

# Stop services
docker-compose down

# View logs
docker-compose logs -f consumer
docker-compose logs -f dashboard

# Restart a service
docker-compose restart consumer

# Rebuild images
docker-compose build
```

### Manual Commands

```bash
# Train ML model
python3 log_processor/ml/trainer.py

# Custom log producer rate
python3 log_processor/producer/sample_app.py --rate 50

# Custom database path
python3 log_processor/consumer/log_consumer.py \
    --kvstore-path /custom/path/logs.db \
    --rabbitmq-host localhost
```

## Troubleshooting

### RabbitMQ Connection Failed

```bash
# Check if RabbitMQ is running
docker ps | grep rabbitmq

# Restart RabbitMQ
docker restart rabbitmq
```

### KV Store Build Failed

```bash
# Install missing dependencies
sudo apt-get install -y cmake g++ python3-dev

# Clean and rebuild
cd kvstore
rm -rf build
mkdir build && cd build
cmake ..
make
```

### Python Module Not Found

```bash
# Reinstall dependencies
pip3 install -r requirements.txt

# Build Python bindings
cd kvstore
python3 -m pip install .
```

### Dashboard Not Loading

```bash
# Check if Flask is installed
pip3 install Flask flask-cors

# Run with debug mode
python3 dashboard/app.py --debug
```

## What's Next?

### Customize the System

1. **Add Your Own Log Sources**: Edit `log_processor/producer/` to ingest real logs
2. **Tune ML Model**: Adjust contamination rate in `ml/anomaly_detector.py`
3. **Customize Dashboard**: Edit templates and static files in `dashboard/`
4. **Scale Up**: Increase RabbitMQ consumers, add Redis caching

### Run Tests

```bash
# C++ tests
make test-cpp

# Python tests
make test-python

# Coverage report
pytest --cov=log_processor --cov-report=html
```

### Performance Tuning

```bash
# Benchmark different configurations
python3 scripts/benchmark_kvstore.py --num-ops 100000

# Monitor resources
docker stats

# Check KV store stats
make stats
```

## Architecture Overview

```
┌─────────────────┐      ┌──────────────────┐      ┌─────────────────┐
│  Log Producers  │─────▶│  RabbitMQ Queue  │─────▶│  Log Consumer   │
│ (Sample Apps)   │      │                  │      │   (Python)      │
└─────────────────┘      └──────────────────┘      └─────────────────┘
                                                            │
                                                            ▼
                         ┌──────────────────────────────────────────┐
                         │      Custom KV Store (C++)               │
                         │  - LSM-tree architecture                 │
                         │  - Memtable (Skip List)                  │
                         │  - SSTable (Disk)                        │
                         │  - Write-Ahead Log                       │
                         │  - Bloom Filters                         │
                         └──────────────────────────────────────────┘
                                          │
                    ┌─────────────────────┼─────────────────────┐
                    ▼                     ▼                     ▼
           ┌────────────────┐   ┌─────────────────┐   ┌──────────────┐
           │  ML Anomaly    │   │  Web Dashboard  │   │  Metrics     │
           │  Detection     │   │  (Flask)        │   │  (Prometheus)│
           └────────────────┘   └─────────────────┘   └──────────────┘
```

## Support

- **Issues**: Open an issue on GitHub
- **Discussions**: Join GitHub Discussions
- **Documentation**: See full README.md

## License

MIT License - see LICENSE file for details
