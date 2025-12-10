import uasyncio
import usocket
import json
import ussl

class stock:
    def __init__(self):
        self.host = "api.upbit.com"
        self.s = None
        self.reader = None
        self.writer = None

    async def connect(self):
        """Initializes the socket connection."""
        if self.s:
            # The connection is already established.
            return

        try:
            addr = usocket.getaddrinfo(self.host, 443)[0][-1]
            
            s = usocket.socket()
            s.setblocking(False)
            s.connect(addr)
            await uasyncio.sleep_ms(200) # Wait for connection
            
            self.s = ussl.wrap_socket(s, server_hostname=self.host)
            self.reader = uasyncio.StreamReader(self.s)
            self.writer = uasyncio.StreamWriter(self.s)

        except Exception as e:
            print(f"Failed to connect: {e}")
            await self.close()
            raise

    async def close(self):
        """Closes the socket connection."""
        if not self.s:
            return

        try:
            self.writer.close()
            await self.writer.wait_closed()
        except Exception as e:
            print(f"Error closing connection: {e}")
        finally:
            self.s = None
            self.reader = None
            self.writer = None

    async def get_price(self, code):
        """Gets the current price from Upbit. Assumes connection is established."""
        if not self.s:
            print("Connection not established. Call connect() first.")
            return None
        
        try:
            path = f"/v1/ticker?markets={code}"
            
            # NOTE: Upbit closes the connection after the response,
            # so a new connection is needed for each call.
            request = f"GET {path} HTTP/1.1\r\nHost: {self.host}\r\nConnection: close\r\n\r\n"
            
            self.writer.write(request.encode('utf-8'))
            await self.writer.drain()

            response = await self.reader.read(-1)
            response_str = response.decode('utf-8')
            
            header_end = response_str.find('\r\n\r\n')
            if header_end == -1:
                print("Invalid HTTP response")
                return None
                
            body = response_str[header_end+4:]
            data = json.loads(body)

            return data[0]['trade_price']

        except Exception as e:
            print(f"Communication error: {e}")
            # Connection is likely dead, so clean up
            await self.deinit()
            return None
        finally:
            # Since the connection is closed by the server, ensure we clean up our side.
            if self.s:
                await self.deinit()