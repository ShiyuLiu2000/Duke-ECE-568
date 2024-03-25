This project involves the creation of an HTTP caching proxy server capable of managing GET, POST, and CONNECT requests.

Concurrency features have been implemented to handle requests originating from various endpoints. TCP sockets are utilized for sending and receiving packets.

Caching of responses follows the validation and expiration rules outlined in RFC7234.

Deployment is achieved through Docker. To run the application, navigate to the docker-deploy folder and execute the following command:

Copy code
sudo docker-compose up
Adjust your web browser's proxy settings to point to this proxy. With the proxy configured, you can browse web pages through it.

Test cases are available in testcases.txt, and danger_log.txt contains additional information about the program.