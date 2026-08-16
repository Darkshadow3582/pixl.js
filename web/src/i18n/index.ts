import i18n from 'i18next'
import { initReactI18next } from 'react-i18next'
import LanguageDetector from 'i18next-browser-languagedetector'

import en from './en.json'
import zhHans from './zh-Hans.json'
import zhTW from './zh-TW.json'
import es from './es.json'
import ru from './ru.json'
import de from './de.json'
import sv from './sv.json'
import ja from './ja.json'

i18n
  .use(LanguageDetector)
  .use(initReactI18next)
  .init({
    resources: {
      en: { translation: en },
      'zh-CN': { translation: zhHans },
      'zh-TW': { translation: zhTW },
      es: { translation: es },
      ru: { translation: ru },
      de: { translation: de },
      sv: { translation: sv },
      ja: { translation: ja },
    },
    fallbackLng: 'zh-CN',
    detection: {
      order: ['cookie', 'navigator'],
      caches: ['cookie'],
      cookieMinutes: 525600,
    },
    interpolation: {
      escapeValue: false,
    },
  })

export default i18n
