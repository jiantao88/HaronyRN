/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { TurboModule } from '@rnoh/react-native-openharmony/ts';
import emitter from '@ohos.events.emitter';
import { preferences } from '../entryability/EntryAbility'
import { BusinessError } from '@ohos.base';

export interface TurboModuleEventData {
  param: string
}

export class SampleTurboModule extends TurboModule {
  pushStringToHarmony(arg: string, id?: number): string {
    if (id) {
      let data: TurboModuleEventData = { param: arg }
      emitter.emit({ eventId: id }, { data: data })
    }
    return arg;
  }

  registerFunction(onComplete: (value: string) => void): void {
    console.log(`RNOH SampleTurboModule::registerFunction + trigger`);
    setTimeout(() => {
      onComplete?.('... from native side');
    }, 1000);
  }

  doAsyncJob(shouldResolve: boolean): Promise<string> {
    return new Promise((resolve, reject) => {
      setTimeout(() => {
        if (shouldResolve) {
          resolve('resolved on native side');
        } else {
          reject('rejected on native side');
        }
      }, 100);
    });
  }

  rnLog(info: string) {
    console.log("ReactNative.log: " + info)
  }

  getPreferencesData(key: string): string {
    if (!preferences) return 'null';
    try {
      let val = preferences.getSync(key, '默认值') as string
      return val
    } catch (err) {
      return '未赋值'
    }
  }

  putPreferencesData(key: string, value: string) {
    if (!preferences) return;
    preferences.putSync(key, value)
    try {
      preferences.flush((err: BusinessError) => {
        if (err) {
          console.error('Failed to flush')
          return;
        }
        console.info('Succeeded in flushing.')
      })
    } catch (err) {
      console.error('Failed to flush')
    }
  }
}