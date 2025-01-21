import asyncio
import websockets
import time
from datetime import datetime

async def handler(websocket):
    async def send_time_messages():
        try:
            while True:
                # await asyncio.sleep(0.01)
                # 500000 : 2000 ope/sec
                # 600000 : 1666 ope/sec
                # 700000 : 1428 ope/sec
                # 800000 : 1250 ope/sec
                # 1000000 : 1000 ope/sec
                # 100000000 : 10 ope/sec
                time_start = time.perf_counter_ns()
                while time.perf_counter_ns() - time_start < 100000000:
                    pass

                dt_now = datetime.now()
                current_time = dt_now.strftime("%Y-%m-%dT%H:%M:%S.%fZ")
                current_time_num = str(int(dt_now.timestamp()*1000000))
                message = '{"did":"did:plc:abcdefghijklmnopqrstuvwx","time_us":'+ current_time_num + ',"kind":"commit","commit":{"rev":"3lfwzhyb5h32h","operation":"create","collection":"app.bsky.feed.like","rkey":"3lfwzhyaspd2h","record":{"$type":"app.bsky.feed.like","createdAt":"' + current_time + '","subject":{"cid":"bafyreiesj5r2jaap5ptctioibtwl52arma2s3chxcyn2nx6mvxpvhaaaaa","uri":"at://did:plc:xyz1234567890abcdefghijk/app.bsky.feed.post/3lfwp6fsszs2p"}},"cid":"bafyreih7grkiydugx4cnbgvuqkimkzhutitn47xmpftkzrb4qzt3saaaaa"}}'
                # message = '{"did":"did:plc:abcdefghijklmnopqrstuvwx","time_us":'+ current_time_num + ',"kind":"commit","commit":{"rev":"3lfwzhy3w7k2x","operation":"create","collection":"app.bsky.graph.follow","rkey":"3lfwzhy3ngc2x","record":{"$type":"app.bsky.graph.follow","createdAt":"' + current_time + '","subject":"did:plc:xyz1234567890abcdefghijk"},"cid":"bafyreihocxl24i44pmxy5fayaet7knz2in3fpp6kquefnehv6biajaaaaa"}}'
                await websocket.send(message)
                # print(f"Sent: {current_time} {current_time_num}")
        except websockets.ConnectionClosed:
            print(f"Connection with {websocket.remote_address} closed.")

    try:
        await asyncio.gather(send_time_messages())
    except websockets.ConnectionClosed:
        print(f"Client {websocket.remote_address} disconnected.")

async def main():
    async with websockets.serve(handler, "localhost", 19283):
        print("WebSocket server started on ws://localhost:19283")
        await asyncio.Future()

if __name__ == "__main__":
    asyncio.run(main())
