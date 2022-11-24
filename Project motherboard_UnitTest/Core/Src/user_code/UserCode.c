#include "UserCode.h"

void LED_ON(LED led) {
  switch (led) {
  case LED_PANEL:
	HAL_GPIO_WritePin(LED_Panel_GPIO_Port, LED_Panel_Pin, GPIO_PIN_SET);
	break;
  case LED_BUILTIN:
	HAL_GPIO_WritePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin, GPIO_PIN_SET);
	break;
  default:
	break;
  }
}
void LED_OFF(LED led) {
  switch (led) {
  case LED_PANEL:
	HAL_GPIO_WritePin(LED_Panel_GPIO_Port, LED_Panel_Pin, GPIO_PIN_RESET);
	break;
  case LED_BUILTIN:
	HAL_GPIO_WritePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin, GPIO_PIN_RESET);
	break;
  default:
	break;
  }
}
void LED_TOGGLE(LED led) {
  switch (led) {
  case LED_PANEL:
	HAL_GPIO_TogglePin(LED_Panel_GPIO_Port, LED_Panel_Pin);
	break;
  case LED_BUILTIN:
	HAL_GPIO_TogglePin(LED_BUILTIN_GPIO_Port, LED_BUILTIN_Pin);
	break;
  default:
	break;
  }
}

void FLASH_WritePage(uint32_t startPage, uint32_t endPage, uint32_t data) {
  HAL_FLASH_Unlock();
  FLASH_EraseInitTypeDef EraseInit;
  EraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInit.PageAddress = startPage;
  EraseInit.NbPages = (endPage - startPage) / FLASH_PAGE_SIZE;
  uint32_t PageError = 0;
  HAL_FLASHEx_Erase(&EraseInit, &PageError);
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, startPage, data);
  HAL_FLASH_Lock();
}
uint32_t FLASH_ReadData(uint32_t addr) {
  /*
   * Read 4 bytes
   * */
  uint32_t data;
  data = *(__IO uint32_t*) (addr);
  return data;
}
