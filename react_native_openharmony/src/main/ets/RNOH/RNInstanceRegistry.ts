import font from '@ohos.font';
import type { RNInstance, RNInstanceOptions } from './RNInstance';
import { RNInstanceImpl } from './RNInstance';
import type { NapiBridge } from './NapiBridge';
import type { RNOHContext } from './RNOHContext';
import type { RNOHLogger } from './RNOHLogger';
import type { DevToolsController } from './DevToolsController';
import { HttpClientProvider } from './HttpClientProvider';
import { HttpClient } from '../HttpClient/ts';
import resourceManager from '@ohos.resourceManager';

export class RNInstanceRegistry {
  private instanceMap: Map<number, RNInstanceImpl> = new Map();

  constructor(
    private logger: RNOHLogger,
    private napiBridge: NapiBridge,
    private devToolsController: DevToolsController,
    private createRNOHContext: (rnInstance: RNInstance) => RNOHContext,
    private httpClientProvider: HttpClientProvider,
    private defaultHttpClient: HttpClient | undefined, // TODO: remove "undefined" when HttpClientProvider is removed
    private resourceManager: resourceManager.ResourceManager,
  ) {
  }

  public async createInstance(options: RNInstanceOptions): Promise<RNInstance> {
    const id = this.napiBridge.getNextRNInstanceId();
    if (options.enableBackgroundExecutor) {
      this.logger.warn("'enableBackgroundExecutor' feature flag is deprecated");
    }
    if (options.enableCAPIArchitecture && !options.fontResourceByFontFamily) {
      this.logger.warn("No custom fonts registered");
    }
    const fontFamilyNameByFontPathRelativeToRawfileDir: Record<string, string> = {}
    for (const [fontFamily, fontResource] of Object.entries(options.fontResourceByFontFamily ?? {})) {
      fontFamilyNameByFontPathRelativeToRawfileDir[fontFamily] = fontResource.params[0]
      font.registerFont({ familyName: fontFamily, familySrc: fontResource })
    }
    const instance = new RNInstanceImpl(
      id,
      this.logger,
      this.napiBridge,
      this.getDefaultProps(),
      this.devToolsController,
      this.createRNOHContext,
      options.enableDebugger ?? false,
      options.enableBackgroundExecutor ?? false,
      options.enableNDKTextMeasuring ?? false,
      options.enableImageLoader ?? false,
      options.enableCAPIArchitecture ?? false,
      options.assetsDest,
      this.resourceManager,
      options.arkTsComponentNames,
      fontFamilyNameByFontPathRelativeToRawfileDir,
      this.httpClientProvider,
      options?.httpClient ?? this.defaultHttpClient,
      options.backPressHandler,
    );
    await instance.initialize(options.createRNPackages({}));
    this.instanceMap.set(id, instance);
    return instance;
  }

  public getInstance(id: number): RNInstance {
    return this.instanceMap.get(id);
  }

  public deleteInstance(id: number): boolean {
    if (this.instanceMap.has(id)) {
      this.instanceMap.delete(id);
      return true;
    }
    return false;
  }

  public forEach(cb: (rnInstance: RNInstanceImpl) => void) {
    this.instanceMap.forEach(cb);
  }

  private getDefaultProps(): Record<string, any> {
    return { concurrentRoot: true };
  }
}
