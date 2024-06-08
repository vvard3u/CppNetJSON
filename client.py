# Imports
import socket
import json
import sys
import argparse
import logging
import configparser

# Enabling logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

# Setting variables from config file
parser = configparser.ConfigParser()
parser.read('./server.cfg')
try:
    HOST = parser.get('DEFAULT', 'host')
    PORT = parser.getint('DEFAULT', 'port')
    BUFFER_SIZE = parser.getint('DEFAULT', 'buffer_size')
except configparser.NoOptionError as e:
    logging.error(f"Missing option in config file: {e}")
    exit(1)
except configparser.Error as e:
    logging.error(f"Error reading config file: {e}")
    exit(1)

# Request sample
def send_request(command, params):
    request = {
        "command1": command,
        "params": params
    }

# Connection to the server
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((HOST, PORT))
            s.sendall(json.dumps(request).encode('utf-8'))
            response_data = b""
            while True:
                part = s.recv(BUFFER_SIZE)
                response_data += part
                if len(part) < BUFFER_SIZE:
                    break
            return json.loads(response_data.decode('utf-8'))
    except (ConnectionRefusedError, ConnectionResetError, socket.timeout) as e:
        logging.error(f"Connection error: {e}")
        return {"error": "Connection error"}
    except Exception as e:
        logging.error(f"Unexpected error: {e}")
        return {"error": "Unexpected error"}


# Main function with args parsing
if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('command', help='CheckLocalFile or QuarantineLocalFile')
    arg_parser.add_argument('params', nargs='*',  help='Parameters for the command in the format key=value')
    args = arg_parser.parse_args()

    command = args.command
    params = {}
    for param in args.params:
        try:
            key, value = param.split('=')
            params[key] = value
        except ValueError:
            logging.error(f"Invalid parameter format: {param}. Expected format key=value")
            sys.exit(1)

    response = send_request(command, params)

    print(json.dumps(response))

