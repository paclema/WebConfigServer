#ifndef WebConfigNetworkObserver_H
#define WebConfigNetworkObserver_H

class WebConfigNetwork;

class WebConfigNetworkObserver {

public:
    // virtual ~WebConfigNetworkObserver() {};
    virtual void onNetworkConnected(void) = 0;
    virtual void onNetworkDisconnected(void) = 0;
};

#endif
