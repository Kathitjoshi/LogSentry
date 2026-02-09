import smtplib, requests, logging
from email.mime.text import MIMEText

logger = logging.getLogger(__name__)

def send_email(config, subject, message):
    try:
        msg = MIMEText(message)
        msg['Subject'] = subject
        msg['From'] = config['from_address']
        msg['To'] = ', '.join(config['to_addresses'])
        
        with smtplib.SMTP(config['smtp_server'], config['smtp_port']) as server:
            server.starttls()
            server.login(config['username'], config['password'])
            server.send_message(msg)
        
        logger.info(f"Email sent: {subject}")
        return True
    except Exception as e:
        logger.error(f"Failed to send email: {e}")
        return False

def send_webhook(url, message):
    try:
        payload = {'text': message}
        response = requests.post(url, json=payload)
        
        if response.status_code == 200:
            logger.info("Webhook notification sent")
            return True
        else:
            logger.error(f"Webhook failed: {response.status_code}")
            return False
    except Exception as e:
        logger.error(f"Failed to send webhook: {e}")
        return False
