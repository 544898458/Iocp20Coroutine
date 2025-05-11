from __future__ import annotations
import asyncio, ssl, websockets
from msgpack import unpackb

async def handle_connection(websocket0) -> None:
    #async with websockets.connect('wss://rtsgame.online:12348') as websocket1:
    async with websockets.connect('wss://test.rtsgame.online:12348') as websocket1:
        async for request in websocket0:
            print('request', unpackb(request))
            await websocket1.send(request)
            reply = await websocket1.recv()
            print('reply', unpackb(reply))
            await websocket0.send(reply)

async def main() -> None:
    ssl_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    ssl_context.load_cert_chain('rtsgame.online-cert.crt', 'rtsgame.online-private.key')
    #ssl_context.load_cert_chain('test.rtsgame.online-cert.crt', 'test.rtsgame.online-private.key')
    async with websockets.serve(handle_connection, '0.0.0.0', 12348, ssl = ssl_context):
        print('server started...')
        await asyncio.Future()

asyncio.run(main())