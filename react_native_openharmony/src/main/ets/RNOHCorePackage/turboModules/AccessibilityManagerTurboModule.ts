import accessibility from '@ohos.accessibility';

import type { TurboModuleContext } from '../../RNOH/TurboModule';
import { TurboModule } from '../../RNOH/TurboModule';
import { BusinessError } from '@ohos.base';
import settings from '@ohos.settings';

const ALL_ABILITY_TYPE: accessibility.AbilityType = 'all';
const ENABLE_ABILITY_STATE: accessibility.AbilityState = 'enable';

export class AccessibilityManagerTurboModule extends TurboModule {
  public static readonly NAME = 'AccessibilityManager';

  public ctx: TurboModuleContext;

  constructor(ctx: TurboModuleContext) {
    super(ctx);
    this.ctx = ctx;
    this.subscribeListeners();
  }

  public setAccessibilityFocus(compId: string) {
    this.ctx.logger.info(`SetAccessibilityFocus reactTag : ` + compId);
    // CAPI not support get component element Id ,use customId instead
    let eventInfo: accessibility.EventInfo = ({
      type: 'requestFocusForAccessibility',
      bundleName: 'com.rnoh.tester',
      triggerAction: 'common',
      customId: compId
    });

    accessibility.sendAccessibilityEvent(eventInfo).then(() => {
      this.ctx.logger.info(`Succeeded in send event,eventInfo is ${eventInfo}`);
    }).catch((err: BusinessError) => {
      this.ctx.logger.error(`failed to send event , Code is ${err.code}, message is ${err.message}`);
    });
  }

  public getCurrentBoldTextState(
    resolveCallback: (isBoldTextEnabled: boolean) => void,
  ) {
    let scale = settings.getValueSync(this.ctx.uiAbilityContext,"font_weight_scale", "0", settings.domainName.USER_PROPERTY);
    this.ctx.logger.info(`Accessibility getCurrentBoldTextState scale :` + scale);
    resolveCallback(parseFloat(scale)>1);
  }

  public announceForAccessibility(announcement: string) {
    this.ctx.logger.error(`announceForAccessibility announcement : ` + announcement);
    let eventInfo: accessibility.EventInfo = ({
      type: 'announceForAccessibility',
      bundleName: 'com.rnoh.tester',
      triggerAction: 'common',
      textAnnouncedForAccessibility: announcement
    });

    accessibility.sendAccessibilityEvent(eventInfo).then(() => {
      this.ctx.logger.info(`hyj Succeeded in send event,eventInfo is ${eventInfo}`);
    }).catch((err: BusinessError) => {
      this.ctx.logger.error(`failed to send event , Code is ${err.code}, message is ${err.message}`);
    });
  }


  public isTouchExplorationEnabled(
    resolveCallback: (isScreenReaderEnabled: boolean) => void,
  ) {
    const isScreenReaderEnabled = accessibility.isOpenTouchGuideSync();
    resolveCallback(isScreenReaderEnabled);
  }

  public isAccessibilityServiceEnabled(
    resolveCallback: (isAccessibilityServiceEnabled: boolean) => void,
  ) {
    accessibility.getAccessibilityExtensionList(ALL_ABILITY_TYPE, ENABLE_ABILITY_STATE)
      .then((data: accessibility.AccessibilityAbilityInfo[]) => {
        if (data.length > 0) {
          resolveCallback(true);
        } else {
          resolveCallback(false);
        }
      }).catch((err: BusinessError) => {
      this.ctx.logger.error(`Failed to get accessibility extension list, Code is ${err.code}, message is ${err.message}`);
    });
  }

  private subscribeListeners() {
    this.ctx.logger.info(`AccessibilityInfoTurboModule subscribeListeners`);
    accessibility.on("accessibilityStateChange" , isOn => {
      this.ctx.logger.info(`AccessibilityInfoTurboModule accessibilityStateChange : ` + isOn);
      this.ctx.rnInstance.emitDeviceEvent("accessibilityServiceChanged", isOn);
    })
  }
}
