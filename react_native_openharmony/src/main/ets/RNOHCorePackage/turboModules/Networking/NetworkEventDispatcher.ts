import { RNInstance } from '../../../RNOH/ts';

export class NetworkEventsDispatcher {
  constructor(private rnInstance: RNInstance) {
  }

  dispatchDidReceiveNetworkData(requestId: number, data: string | Object) {
    this.sendEvent("didReceiveNetworkData", [requestId, data])
  }

  dispatchDidReceiveNetworkResponse(requestId: number, status: number, headers: Object, url: string) {
    this.sendEvent("didReceiveNetworkResponse", [requestId, status, headers, url])
  }

  dispatchDidCompleteNetworkResponse(requestId: number) {
    this.sendEvent("didCompleteNetworkResponse", [requestId, ""])
  }

  dispatchDidCompleteNetworkResponseWithError(requestId: number, error: string, timeout: boolean) {
    this.sendEvent("didCompleteNetworkResponse", [requestId, error, timeout])
  }

  dispatchDidSendNetworkData(requestId: number, progress: number, total: number) {
    this.sendEvent("didSendNetworkData", [requestId, progress, total])
  }

  dispatchDidReceiveNetworkDataProgress(requestId: number, progress: number, total: number) {
    this.sendEvent("didReceiveNetworkDataProgress", [requestId, progress, total])
  }

  dispatchDidReceiveNetworkIncrementalData(requestId: number, data: string, progress: number, total: number) {
    this.sendEvent("didReceiveNetworkIncrementalData", [requestId, data, progress, total])
  }

  private sendEvent(eventName: string, body: Object) {
    this.rnInstance.emitDeviceEvent(eventName, body)
  }
}
