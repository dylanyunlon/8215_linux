# ATCUpdateService

## ATCUpdateServcie Version

### Version 1.0.0
- initial version
  - support AB Upgrade

### Version 1.1.0
- Use ATC_AB_UPGRADE to control complication of atcupdateservice
- fix the realsize of the image is not changed
- change the api of atcupdateclient

### Version 1.2.0
- support AB Upgrade when AVB enable
- refactor the flow of upgrade
- use exception to main loop instead of put OnError Action into main loop directly
- add Image serial class and File class