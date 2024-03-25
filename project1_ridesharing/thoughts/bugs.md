

- Done司机接单以后应该跳转到status_driver面（现在是停留在搜索页面
- Done司机和普通rider的signup界面都加返回到login view的按钮
- Done司机和rider的search view搜不到订单时，显示一行字“no ride available”
- done search view加返回到status view的button，司机和乘客的search都是
- done 乘客下单时total passenger人数实际上比用户填的多1





- done driver注册时vehicle type不可以是none（但现在可以选None

- done 用户request ride时Vehicle type应该默认可以不填，不填的时候应该是所有type的车都可以符合要求（但现在是要求user必须填vehicle type



- done 乘客不可以下过去时间的订单（但现在可以
- done 乘客下单时乘客数不可以选0（但现在可以
- done 司机注册时capacity不可以选0（但现在可以
- Done 司机不可以接乘客数大于自己capacity的订单（但现在可以
- done 司机不可以接不符合自己vehicle type的单（但现在可以
- done 司机不可以接不符合自己special request info的单（但现在可以



- done 司机profile界面要加编辑是否is_active_driver的checkbox
- Done 司机profile界面加一个返回status_driver的按钮
- Done  在司机和user主页都加一个logout按钮，logout session，再回到login界面
- done 司机和rider的搜索页都加前后10min的time window





#### Join(done)

正常，userA出了一个订单之后，另外一个userB要去搜符合自己要求的可以join的订单时

- 如果userA下单时没勾shareable，那就不会在userB那被搜出来，也不能被join
- 如果userA下单时勾了shareable，要能被userB搜到，且在userB点了join之后，userB返回自己的status_rider界面，此时刚被他join的order应该刷新显示出来在这个界面里，并且再去搜订单时，这个订单不应该再被显示出来

现在：

- userA可以搜到自己下的订单并且点join，订单的乘客人数会增加，ride_party里和userA关联的人数不变
- userB可以搜到自己已经join的订单并且点join，订单的乘客人数会增加，ride_party里和userB关联的人数不变
- userB点join时，不会弹窗问他有几个人同乘
- userB点完join后，订单不会出现在userB的status_rider界面里，但userA那边看自己对应订单的order detail时，可以看到订单里出现了userB，

