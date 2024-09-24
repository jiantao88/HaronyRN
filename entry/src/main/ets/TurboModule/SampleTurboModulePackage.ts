/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { RNPackage, TurboModulesFactory } from '@rnoh/react-native-openharmony/ts';
import { TurboModule, TurboModuleContext } from '@rnoh/react-native-openharmony/ts';
import { SampleTurboModule } from './SampleTurboModule';
import { SampleTurboModule2 } from './SampleTurboModule2';

const SAMPLE_TURBO_MODULE_CLASS_BY_NAME: Record<string, typeof TurboModule> = {
  [SampleTurboModule.NAME]: SampleTurboModule,
  [SampleTurboModule2.NAME]: SampleTurboModule2,
}

class SampleTurboModulesFactory extends TurboModulesFactory {
  createTurboModule(name: string): TurboModule | null {
    if (this.hasTurboModule(name)) {
      return new SAMPLE_TURBO_MODULE_CLASS_BY_NAME[name](this.ctx);
    }
    return null;
  }

  hasTurboModule(name: string): boolean {
    return name in SAMPLE_TURBO_MODULE_CLASS_BY_NAME;
  }
}

export class SampleTurboModulePackage extends RNPackage {
  createTurboModulesFactory(ctx: TurboModuleContext): TurboModulesFactory {
    return new SampleTurboModulesFactory(ctx);
  }
}