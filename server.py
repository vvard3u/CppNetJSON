# Imports
import socket
import os
import selectors
import signal
import configparser
import json
import logging
from concurrent.futures import ThreadPoolExecutor

# Enabling logging
logging.basicConfig(
    filename='logfile.txt', format='%(asctime)s - %(name)s - %(levelname)s - %(message)s', level=logging.INFO
)

logger = logging.getLogger(__name__)


# Setting variables from config file
parser = configparser.ConfigParser()
parser.read('./server.cfg')

try:
    HOST = parser.get('DEFAULT', 'host')
    PORT = parser.getint('DEFAULT', 'port')
    QUARANTINE_DIR = parser.get('DEFAULT', 'quarantine_dir')
    THREADS = parser.getint('DEFAULT', 'threads')
    BUFFER_SIZE = parser.getint('DEFAULT', 'buffer_size')

except configparser.NoOptionError as e:
    logging.error(f"Missing option in config file: {e}")
    exit(1)
except configparser.Error as e:
    logging.error(f"Error reading config file: {e}")
    exit(1)

# Creating quarantine directory
if not os.path.exists(QUARANTINE_DIR):
    os.makedirs(QUARANTINE_DIR)

sel = selectors.DefaultSelector()


# Connection handler
def conn_handler(conn, addr):
    try:
        data = conn.recv(BUFFER_SIZE)
        if not data:
            return
        try:
            request = json.loads(data.decode('utf-8'))
        except json.JSONDecodeError:
            logger.error(f"Invalid JSON from {addr}")
            conn.sendall(json.dumps({'error': 'Invalid JSON'}).encode('utf-8'))
            return
        logger.info(f"Parsed request: {request}")  
        command = request.get('command1')
        params = request.get('params', {})
        
        if command == 'CheckLocalFile':
            response = check_local_file(params)
        elif command == 'QuarantineLocalFile':
            response = quarantine_local_file(params)
        else:
            response = {'error': 'Unknown command'}

        conn.sendall(json.dumps(response).encode('utf-8'))
        logger.info(f'Data sended to {addr} succesfully.')
    except Exception as e:
        logger.error(f"Error handling request from {addr}: {e}")
    finally:
        conn.close()

# Checking file for signature
def check_local_file(params):
    file_path = params.get('file_path')
    signature = bytes.fromhex(params.get('signature'))

    if not file_path or not signature:
        return {'Error:': 'invalid parameters'}
    if not os.path.exists(file_path):
        return {'Error:': 'file not found'}
    if os.path.isdir(file_path):
        return {'Error:' : 'File is directory'}
    
    offsets = []
    with open(file_path, 'rb') as f:
        content = f.read()
        offset = content.find(signature)
        while offset != -1:
            offsets.append(offset)
            offset = content.find(signature, offset + 1)
    return {'Offsets:': offsets} # Returning the signature offset/where it was found


# Moving file to quarantine directory
def quarantine_local_file(params):
    file_path = params.get('file_path')

    if not file_path:
        return {'Error:' : 'invalid parameters'}
    if not os.path.exists(file_path):
        return {'Error:' : 'file not found'}
    if os.path.isdir(file_path):
        return {'Error:' : 'File is directory'}
    file_name = os.path.basename(file_path)
    quarantine_path = os.path.join(QUARANTINE_DIR, file_name)
    
    try:
        os.rename(file_path, quarantine_path)
        logger.info(f'{file_name} placed to quarantine')
        return {'Status': 'File replaced to quarantine.'}
    except Exception as e:
        logger.error(f'Error: {e}')
        return {'Error': str(e)}

    
    
# Accepting connections
def accept_wrapper(sock):
    conn, addr = sock.accept()
    logger.info(f'Connected by {addr}')
    conn.setblocking(False)
    sel.register(conn, selectors.EVENT_READ, read_wrapper)

# Setting multithreading
def read_wrapper(conn):
    sel.unregister(conn)
    with ThreadPoolExecutor(max_workers=THREADS) as executor:
        executor.submit(conn_handler, conn, conn.getpeername())


# Setting server with variables and start it.
def start_server():
    lsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    lsock.bind((HOST, PORT))
    lsock.listen()
    lsock.setblocking(False)
    sel.register(lsock, selectors.EVENT_READ, accept_wrapper)
    print(f'Server listening on {HOST}:{PORT}')
    logger.info(f'Server listening on {HOST}:{PORT}')
    
    while True:
        events = sel.select(timeout=None)
        for key, mask in events:
            callback = key.data
            callback(key.fileobj)

# Maintaining CTRL+C signal.
def signal_handler(sig, frame):
    print('Shutting down server...')
    logger.info('Shutting down server...')
    sel.close()
    os._exit(0)

signal.signal(signal.SIGINT, signal_handler)



if __name__=="__main__":
    start_server()