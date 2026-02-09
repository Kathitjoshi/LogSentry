# Quick Start Guide - Local Development (Windows/Linux/Mac)

This guide will help you run the Distributed Log Analyzer **locally without Docker** on Windows, Linux, or Mac.

## Prerequisites

- Python 3.9+ installed
- Git (optional, for cloning)
- Terminal/PowerShell access

## Step 1: Install Dependencies

### Windows (PowerShell):
```powershell
# Create virtual environment
python -m venv .venv

# Activate virtual environment
.\.venv\Scripts\Activate.ps1

# Install dependencies
pip install -r requirements.txt
```

### Linux/Mac (Bash):
```bash
# Create virtual environment
python3 -m venv .venv

# Activate virtual environment
source .venv/bin/activate

# Install dependencies
pip install -r requirements.txt
```

## Step 2: Start RabbitMQ

You have two options:

### Option A: Using Docker (Recommended)
```bash
docker run -d --name rabbitmq \
  -p 5672:5672 \
  -p 15672:15672 \
  -e RABBITMQ_DEFAULT_USER=admin \
  -e RABBITMQ_DEFAULT_PASS=admin123 \
  rabbitmq:3-management
```

### Option B: Install RabbitMQ Locally
- Windows: Download from https://www.rabbitmq.com/download.html
- Linux: `sudo apt-get install rabbitmq-server`
- Mac: `brew install rabbitmq`

Then start the service and create user:
```bash
# Start RabbitMQ
sudo systemctl start rabbitmq-server  # Linux
brew services start rabbitmq          # Mac

# Create admin user
rabbitmqctl add_user admin admin123
rabbitmqctl set_permissions -p / admin ".*" ".*" ".*"
```

## Step 3: Start the Services

Open **3 separate terminal windows** and activate the virtual environment in each.

### Terminal 1 - Log Producer (generates sample logs):
```powershell
# Windows
cd log-producers
python sample_web_app.py
```

```bash
# Linux/Mac
cd log-producers
python3 sample_web_app.py
```

You should see output like:
```
[web-server] Connected to RabbitMQ
[web-server] Starting log generation
[web-server] INFO: GET /api/users - 200 - 45ms
...
```

### Terminal 2 - Log Processor (processes logs):
```powershell
# Windows
cd log-processor
python src\processor.py
```

```bash
# Linux/Mac
cd log-processor
python3 src/processor.py
```

You should see:
```
INFO - LogProcessor initialized
INFO - Connected to RabbitMQ at localhost:5672
INFO - Waiting for messages...
```

### Terminal 3 - Dashboard (web interface):
```powershell
# Windows
cd dashboard
python app.py
```

```bash
# Linux/Mac
cd dashboard
python3 app.py
```

You should see:
```
 * Running on http://0.0.0.0:5000
```

## Step 4: Access the Dashboard

Open your browser and go to:
- **Dashboard**: http://localhost:5000
- **RabbitMQ Management**: http://localhost:15672 (login: admin/admin123)

## Troubleshooting

### Issue: "ModuleNotFoundError: No module named 'pika'"
**Solution**: Make sure you activated the virtual environment and installed requirements:
```bash
pip install -r requirements.txt
```

### Issue: "Connection refused" to RabbitMQ
**Solution**: 
1. Check if RabbitMQ is running: `docker ps` (if using Docker)
2. Try connecting to http://localhost:15672 in your browser
3. Restart RabbitMQ container:
   ```bash
   docker restart rabbitmq
   ```

### Issue: "FileNotFoundError: config/config.yaml"
**Solution**: The config file path is now automatically detected. If you still get this error:
1. Make sure you're running the scripts from the correct directory
2. Check that `config/config.yaml` exists in the project root

### Issue: Dashboard shows "Error loading data"
**Solution**: 
1. The KV store server may not be running (it's optional for testing)
2. The dashboard will work with mock data if KV store is unavailable
3. Check that log-processor is running and processing messages

### Issue: No logs appearing in dashboard
**Solution**:
1. Verify the log producer is running and generating logs
2. Check RabbitMQ management UI to see if messages are being queued
3. Ensure log processor is consuming messages (check Terminal 2 output)

## Testing Individual Components

### Test RabbitMQ Connection:
```python
import pika

credentials = pika.PlainCredentials('admin', 'admin123')
connection = pika.BlockingConnection(
    pika.ConnectionParameters('localhost', 5672, '/', credentials)
)
print("Connected successfully!")
connection.close()
```

### Test Log Producer Only:
```bash
cd log-producers
python sample_web_app.py
```

Then check RabbitMQ management UI (http://localhost:15672) to see if messages are being queued.

### View Logs in RabbitMQ:
1. Go to http://localhost:15672
2. Login with admin/admin123
3. Click on "Queues" tab
4. Click on "logs" queue
5. You should see messages being produced

## Next Steps

Once everything is working locally:

1. **Explore the Dashboard**: Check out different visualizations
2. **Try Different Log Patterns**: Modify the sample producer to generate different types of logs
3. **Test Anomaly Detection**: The ML model learns from normal logs and flags anomalies
4. **Scale Up**: Run multiple producers or processors simultaneously

## Running with Docker Compose

If you prefer to use Docker for everything:

```bash
# Build and start all services
docker-compose up --build

# View logs
docker-compose logs -f

# Stop all services
docker-compose down
```

## Common Commands

```bash
# Stop a specific service
Ctrl+C in the terminal window

# Restart RabbitMQ
docker restart rabbitmq

# Clear RabbitMQ queues
# Go to http://localhost:15672 -> Queues -> Delete queue

# Check Python package versions
pip list

# Update dependencies
pip install --upgrade -r requirements.txt
```

## Project Structure for Reference

```
distributed-log-analyzer/
├── config/
│   └── config.yaml          # Configuration file
├── log-producers/
│   ├── sample_web_app.py    # Generates web server logs
│   ├── sample_api_server.py # Generates API logs
│   └── sample_database_app.py # Generates database logs
├── log-processor/
│   ├── src/
│   │   ├── processor.py     # Main processor
│   │   ├── log_parser.py    # Log parsing
│   │   └── kvstore_client.py # KV store client
│   └── ml/
│       └── anomaly_detector.py # ML anomaly detection
├── dashboard/
│   ├── app.py              # Flask web server
│   ├── templates/          # HTML templates
│   └── static/             # CSS, JS
└── requirements.txt         # Python dependencies
```

## Support

If you encounter issues:
1. Check the terminal output for error messages
2. Verify all dependencies are installed
3. Ensure RabbitMQ is accessible
4. Try restarting the services in order: RabbitMQ → Producer → Processor → Dashboard

Good luck! 
