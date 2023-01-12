from zeroconf import ServiceBrowser, ServiceListener, Zeroconf
from time import sleep
import socket

# create a set to store the names of the services
services = {}

class MyListener(ServiceListener):

    def __init__(self, target_name): 
        self.target_name = target_name
        super().__init__()

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        print(f"Service {name} updated")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        print(f"Service {name} removed")

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        info = zc.get_service_info(type_, name)
        if not name.startswith(self.target_name):
            return
        services[name.split('.')[0]] = socket.inet_ntoa(info.addresses[0])
        print(f"Service {name} added, service address: {socket.inet_ntoa(info.addresses[0])}")



def get_baozi_addresses(name = "baozi", timeout=5):
    zeroconf = Zeroconf()
    listener = MyListener(name)
    browser = ServiceBrowser(zeroconf, "_mqtt._tcp.local.", listener)
    sleep(timeout) 

    zeroconf.close()
    print(services)
    return services