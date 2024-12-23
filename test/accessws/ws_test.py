import asyncio
import websockets
import json
import signal
import logging
import sys

# Configure logging
logging.basicConfig(
    level=logging.DEBUG,
    format='%(asctime)s - %(levelname)s - %(message)s'
)

# Global flag for graceful shutdown
running = True
# Global tasks for cleanup
tasks = []

def handle_signal(signum, frame):
    global running
    logging.info("Received signal to terminate")
    running = False
    # Force exit after 3 seconds if program hasn't closed
    if signum in [signal.SIGINT, signal.SIGTERM]:
        sys.exit(0)

async def keep_alive(websocket):
    """Send periodic ping messages to keep the connection alive"""
    while running:
        try:
            ping_msg = {"method": "server.ping", "params": [], "id": 0}
            logging.debug(f"Sending application ping: {ping_msg}")
            await websocket.send(json.dumps(ping_msg))
            await asyncio.sleep(10)
        except websockets.exceptions.ConnectionClosed:
            logging.error("Connection closed in keep_alive")
            break
        except Exception as e:
            logging.error(f"Keep-alive error: {e}")
            break

async def receive_messages(websocket):
    """Receive and print messages from the WebSocket"""
    while running:
        try:
            response = await websocket.recv()
            if "pong" in response:
                logging.debug(f"Received application pong: {response}")
            else:
                logging.info(f"Received message: {response}")
        except websockets.exceptions.ConnectionClosed as e:
            logging.error(f"Connection closed in receive_messages: {e}")
            break
        except Exception as e:
            logging.error(f"Receive error: {e}")
            break

async def test_websocket():
    uri = "ws://viabtc-nginx:8091"
    logging.info(f"Connecting to {uri}")
    
    try:
        # Create connection with all ping mechanisms disabled
        async with websockets.connect(
            uri,
            ping_interval=None,     # Disable automatic ping
            ping_timeout=None,      # Disable ping timeout
            close_timeout=None,     # Disable close timeout
            max_size=2**20,        # Increase max message size
            compression=None,       # Disable compression
            max_queue=2**10        # Increase message queue size
        ) as websocket:
            logging.info("Connected to WebSocket server")
            
            # Test basic ping first
            ping_msg = {"method": "server.ping", "params": [], "id": 1}
            logging.info(f"Sending initial ping: {ping_msg}")
            await websocket.send(json.dumps(ping_msg))
            response = await websocket.recv()
            logging.info(f"Initial ping response: {response}")
            
            # Subscribe to market data
            subscriptions = [
                {"method": "price.subscribe", "params": ["BTC/USDT"], "id": 2},
                {"method": "depth.subscribe", "params": ["BTC/USDT", 10, "0"], "id": 3},
                {"method": "deals.subscribe", "params": ["BTC/USDT"], "id": 4}
            ]
            
            # Send subscriptions
            for sub in subscriptions:
                logging.info(f"Sending subscription: {sub}")
                await websocket.send(json.dumps(sub))
                response = await websocket.recv()
                logging.info(f"Subscription response: {response}")
            
            # Start keep-alive and receive tasks
            global tasks
            tasks = [
                asyncio.create_task(keep_alive(websocket)),
                asyncio.create_task(receive_messages(websocket))
            ]
            
            # Wait until interrupted
            try:
                await asyncio.gather(*tasks)
            except KeyboardInterrupt:
                logging.info("Received keyboard interrupt")
            finally:
                # Cancel all tasks
                for task in tasks:
                    task.cancel()
                # Wait for tasks to complete
                await asyncio.gather(*tasks, return_exceptions=True)
                logging.info("Tasks cleaned up")
    except Exception as e:
        logging.error(f"Connection error: {e}")

async def main():
    try:
        await test_websocket()
    except KeyboardInterrupt:
        logging.info("Exiting due to keyboard interrupt")
    except Exception as e:
        logging.error(f"Main loop error: {e}")
    finally:
        # Cancel any remaining tasks
        for task in tasks:
            if not task.done():
                task.cancel()
        logging.info("Cleanup complete")

if __name__ == "__main__":
    # Set up signal handlers
    signal.signal(signal.SIGINT, handle_signal)
    signal.signal(signal.SIGTERM, handle_signal)
    
    logging.info("Starting WebSocket test (Press Ctrl+C to exit)")
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logging.info("Exiting...")
    finally:
        logging.info("Program terminated")
