import xml.etree.ElementTree as ET
import xml.dom.minidom
import socket
import time
from multiprocessing import Process

SERVER_ADDRESS = ("vcm-39216.vm.duke.edu", 12345)
NUM_PROCESSES = 10


# this fucntion is to generate xml randomly
def generate_xml(account_id, balance, symbols):
    root = ET.Element("create")
    ET.SubElement(root, "account", id=str(account_id), balance=str(balance))
    for symbol, amount in symbols.items():
        sym_element = ET.SubElement(root, "symbol", sym=symbol)
        account_element = ET.SubElement(sym_element, "account", id=str(account_id))
        account_element.text = str(amount)
    xml_str = ET.tostring(root, encoding="utf8", method="xml")
    pretty_xml_str = xml.dom.minidom.parseString(xml_str).toprettyxml(indent="   ")
    print(pretty_xml_str)
    return format_xml_data(root)


# this function edit the function data
def format_xml_data(element):
    xml_string = ET.tostring(element)
    formatted_data = f"{len(xml_string)}\n{xml_string.decode('utf-8')}"
    return formatted_data.encode("utf-8")


# this function can send request to my virtual machine port 12345
def send_request(account_id, balance, symbols):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect(SERVER_ADDRESS)
        client.sendall(generate_xml(account_id, balance, symbols))


# this function can send request to my virtual machine port 12345
def measure_performance(start_id, num_requests):
    for i in range(num_requests):
        send_request(start_id + i, 99999, {"DUKE": "500", "UNC": "600"})


def run_test(num_requests):
    print(f"\nRunning test for NUM_REQUESTS = {num_requests}")
    start_time = time.time()

    processes = [
        Process(
            target=measure_performance,
            args=(
                i * num_requests,
                num_requests,
            ),
        )
        for i in range(NUM_PROCESSES)
    ]
    for process in processes:
        process.start()
    for process in processes:
        process.join()

    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"Total Time: {elapsed_time}s")
    print(f"Throughput (requests/s): {num_requests * NUM_PROCESSES / elapsed_time}")


def main():
    # for num_requests in [10, 100, 1000]:
    #     run_test(num_requests)
    account_id = 12345
    balance = 100000
    symbols = {"AAPL": 50, "MSFT": 75}
    xml_output = generate_xml(account_id, balance, symbols)
    print(xml_output)


if __name__ == "__main__":
    main()
