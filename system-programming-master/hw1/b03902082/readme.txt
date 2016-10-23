For every loop I use `select` to check if there is any incoming connection on the listening port. If there is any, I try to handle the connection.
Then I use `select` to check if any connections has become unblocked. If there is any, I try to handle it's input.
When handling connections of write_server I lock the corresponding bytes in the file `item_list`. Meantime I keep an array of locked items per process so I can keep track of locked items using the array and linux's advisory lock mechanism.

Since I'm more familiar with cplusplus, I finished the homework with cplusplus instead of c.
