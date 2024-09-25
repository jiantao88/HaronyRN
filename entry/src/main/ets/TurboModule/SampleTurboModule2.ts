/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

import { TurboModule } from '@rnoh/react-native-openharmony/ts';
import promptAction from '@ohos.promptAction'
import axios, { AxiosResponse, AxiosError } from '@ohos/axios'

export interface Fruit {
  name: string,
  color?: string
}

export interface VoidStruct {}

export interface EnglishNovelModel {
  content: string
  tts: string
  note: string
  dateline: string
  img: string
}

export interface ResultModel {
  code: number
  result: EnglishNovelModel
  msg: string
}

export class SampleTurboModule2 extends TurboModule {
  public static readonly NAME = 'SampleTurboModule2';

  getObject(obj: VoidStruct) {
    this.showToast(JSON.stringify(obj))
  }

  getRequest(): Promise<ResultModel> {
    return new Promise((resolve, reject) => {
      let urlStr = 'https://api.oioweb.cn/api/common/OneDayEnglish'
      axios.get<string, AxiosResponse<ResultModel>, null>(urlStr)
        .then((response: AxiosResponse<ResultModel>) => {
          let data: ResultModel = response.data
          let novel: EnglishNovelModel = data.result
          this.showToast(novel.note)
          resolve(data)
        })
        .catch((error: AxiosError) => {
          console.error('SampleTurboModule2.getRequest error', JSON.stringify(error));
          this.showToast(JSON.stringify(error))
          reject(error)
        });
    })
  }

  checkPwd(param: VoidStruct, success: (res: VoidStruct) => void, failure: (res: VoidStruct) => void) {
    console.log(JSON.stringify(param))
    const result: VoidStruct = {}
    const flag: boolean = true
    if (flag) {
      this.showToast('checkPwd success!')
      success(result)
    } else {
      this.showToast('checkPwd failure xx')
      failure(result)
    }
  }

  eatFruit(fruit: Fruit) {
    this.showToast('正在吃水果，' + fruit.name + "很好吃。颜色呢？" + (fruit.color ?? "透明的"))
  }

  test() {
    this.showToast('调用 SampleTurboModule2 的 Test')
  }

  private showToast(msg: string) {
    promptAction.showToast({
      message: msg,
      duration: 2000
    });
  }
}