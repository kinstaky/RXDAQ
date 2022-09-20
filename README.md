# RXDAQ - digital data acquisition system under rpc framework

## description

This is an incomplete project for the digital data acquisition using in nuclear physics under the rpc framework. The purpose is to use the pixie16 firmware (mostly revision F) from xia easily.


## features

 + use the new version api (PixieSDK) from xia llc
 + use gRPC framework to seperate the model part (physical firmware), the visual part and the control part
 + provides 3 modes to control the firmware
   + command mode
   + visual mode(to do)
   + batch mode