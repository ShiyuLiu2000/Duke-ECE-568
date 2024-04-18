xml=$(cat multiple_creation.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat buy_order.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat sell_order.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat partial_order_execution_cancellation.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat non_existent_error.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat match_order.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat further_test.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat further_test1.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat further_test2.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat further_test3.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat further_test4.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

xml=$(cat further_test5.xml)
size=$(echo -n "$xml" | wc -c)
content="$size\n$xml"
echo -e "$content" | nc -w 1 vcm-39324.vm.duke.edu 12345 >> response.xml

