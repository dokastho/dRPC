# dRPC
RPC library built from scratch with love ♥

## Summary

I'm a big admirer of Golang, and often find myself wishing I had the flexibility of gRPC in C++. I discovered some existing options online but ultimately decided that I could make my own version with minimal effort to best fit my use-cases.

## How It Works

The dRPC framework is very similar to that of gRPC in Golang. 

### Client

From a client's perspective, call `drpc_client::Call` with the required arguments to perform an RPC on a remote machine. After receiving a reply from the server, the client will close the connection.

### Server

The `drpc_server` class is stateful: after initialization a server must publish its dRPC hooks using the `drpc_server::publish_endpoint` method. The server will accept the connection from a client and dispatch that execution to another thread. This will always begin from the `drpc_server::stub` private method, which is effectively a callback method for sending a reply after a successful RPC. Stub will invoke the target function, and send the reply once that function has returned.

## Known Limitations

There are two main limitations to the exiting implementation of dRPC.

* it is required to make all RPC endpoints static methods. This is so that the pointer-to-function types can be invoked from the `drpc_server` object after they are published.
* it is currently not possible to send RPC's containing data types with dynamic length, such as vectors or maps. All RPC classes are required to be defined with static-length data types only.

## P.S.

Big shout-out to the community over at [golang/go](https://github.com/golang/go) for maintaining such an awesome language that inspired me to create this library.
