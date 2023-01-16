from zeroconf import ServiceBrowser, ServiceListener, Zeroconf
from time import sleep
import socket

class MyListener(ServiceListener):

    def __init__(self, target_name, services):
        self.target_name = target_name
        self.services = services
        super().__init__()

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        pass

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        info = zc.get_service_info(type_, name)
        if not name.startswith(self.target_name):
            return
        if info is not None and info.properties is not None:
            ip = socket.inet_ntoa(info.addresses[0])
            version = info.properties.get(b"version").decode("utf-8") if b"version" in info.properties else "unknown"
            instance = name.split('.')[0]

            self.services[instance] = {"ip": ip, "version": version}



def get_baozi_addresses(name = "baozi", timeout=5):
    zeroconf = Zeroconf()

    services = {}
    listener = MyListener(name, services)
    browser = ServiceBrowser(zeroconf, "_mqtt._tcp.local.", listener)

    print(f"Searching for {name} devices...", end="", flush=True)
    for i in range(timeout):
        print(".", end="", flush=True)
        sleep(i)

    zeroconf.close()

    print("\n\nfound the following devices:")
    for service, info in services.items():
        print(service, info)
    return services