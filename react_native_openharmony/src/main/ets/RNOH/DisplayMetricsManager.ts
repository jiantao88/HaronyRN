import { DisplayMetrics } from './types';
import window from '@ohos.window';
import { RNOHLogger } from './RNOHLogger';
import display from '@ohos.display';
import { RNOHError } from "./RNOHError"
import UIContext from '@ohos.arkui.UIContext';

const defaultDisplayMetrics: DisplayMetrics = {
  windowPhysicalPixels: {
    width: 0,
    height: 0,
    scale: 1,
    fontScale: 1,
    densityDpi: 480,
  },
  screenPhysicalPixels: {
    width: 0,
    height: 0,
    scale: 1,
    fontScale: 1,
    densityDpi: 480
  },
} as const;

/**
 * @internal
 */
export class DisplayMetricsManager {
  private displayMetrics: DisplayMetrics = defaultDisplayMetrics;
  private fontSizeScale: number
  private logger: RNOHLogger

  constructor(fontSizeScale: number, logger: RNOHLogger) {
    this.fontSizeScale = fontSizeScale;
    this.logger = logger.clone("DisplayMetricsManager");
  }

  public getFoldStatus():display.FoldStatus{
    return display.getFoldStatus()
  }

  public getIsSplitScreenMode():boolean{
    return AppStorage.get("isSplitScreenMode") ?? false
  }

  public getFontSizeScale():number{
    return AppStorage.get("fontSizeScale") ?? this.fontSizeScale
  }

  public updateWindowSize(windowSize: window.Size | window.Rect) {
    this.displayMetrics.windowPhysicalPixels.height = windowSize.height;
    this.displayMetrics.windowPhysicalPixels.width = windowSize.width;
    this.updateDisplayMetrics()
  }

  public updateDisplayMetrics() {
    try {
      const displayInstance = display.getDefaultDisplaySync();
      this.displayMetrics = {
        screenPhysicalPixels: {
          width: displayInstance.width,
          height: displayInstance.height,
          scale: displayInstance.densityPixels,
          fontScale: this.getFontSizeScale(),
          densityDpi: displayInstance.densityDPI,
        },
        windowPhysicalPixels: {
          width: this.displayMetrics.windowPhysicalPixels.width,
          height: this.displayMetrics.windowPhysicalPixels.height,
          scale: displayInstance.densityPixels,
          fontScale: this.getFontSizeScale(),
          densityDpi: displayInstance.densityDPI,
        }
      };
    }
    catch (err) {
      this.logger.error('Failed to update display size ' + JSON.stringify(err));
    }
  }

  public getDisplayMetrics(): DisplayMetrics {
    if (!this.displayMetrics) {
      throw new RNOHError({ whatHappened: "DisplayMetrics are undefined", howCanItBeFixed: [] });
    }
    return this.displayMetrics;
  }
}