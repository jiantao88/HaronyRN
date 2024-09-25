/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { RNPackageContext, RNPackage } from '@rnoh/react-native-openharmony';
import { SampleTurboModulePackage } from '../TurboModule/SampleTurboModulePackage';

export function createRNPackages(ctx: RNPackageContext): RNPackage[] {
  return [
    new SampleTurboModulePackage(ctx)
  ];
}