/*
 * BESM-6 stylized error messages.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your discretion) any later version.
 * See the accompanying file "COPYING" for more details.
 */
#include "gettext.h"

char *errtxt[] = {
	gettext_noop(" Неизвестная ош."),	/*  0 */
	gettext_noop(" КОНЕЦ ЗАДАЧИ"),		/*  1 */
	gettext_noop(" Внутр.ош.эмул"),      	/*  2 */
	gettext_noop(" Пока рано"),          	/*  3 */
	gettext_noop(" Стоп машина!"),       	/*  4 */
	gettext_noop(" Err 5"),              	/*  5 */
	gettext_noop(" Err 6"),              	/*  6 */
	gettext_noop(" Err 7"),              	/*  7 */
	gettext_noop(" Err 8"),              	/*  8 */
	gettext_noop(" Err 9"),              	/*  9 */
	gettext_noop(" Err 10"),             	/* 10 */
	gettext_noop(" Err 11"),             	/* 11 */
	gettext_noop(" Err 12"),             	/* 12 */
	gettext_noop(" Err 13"),             	/* 13 */
	gettext_noop(" ДЕЛЕНИЕ НА НУЛЬ"),    	/* 14 */
	gettext_noop(" ПЕРЕПОЛНЕНИЕ АУ"),    	/* 15 */
	gettext_noop(" ЧИС.В ЧУЖ.ЛИСТЕ"),    	/* 16 */
	gettext_noop(" КОМ.В ЧУЖ.ЛИСТЕ"),    	/* 17 */
	gettext_noop(" ЗАПРЕЩ.КОМАНДА"),     	/* 18 */
	gettext_noop(" СНЯТА ОПЕРАТОРОМ"),   	/* 19 */
	gettext_noop(" КОНТРОЛЬ КОМАНДЫ"),   	/* 20 */
	gettext_noop(" ОСТАНОВ ПО СЧИТ."),   	/* 21 */
	gettext_noop(" ОСТАНОВ ПО ЗАП."),    	/* 22 */
	gettext_noop(" ОСТАНОВ ПО КРА"),     	/* 23 */
	gettext_noop(" ЗАТЕРТО СПО"),        	/* 24 */
	gettext_noop(" ВЫВОД ЗАПРЕЩЕН"),     	/* 25 */
	gettext_noop(" НЕСУЩ.ВИД РАБОТЫ"),   	/* 26 */
	gettext_noop(" СМЕНИЛСЯ ВИД РАБ"),   	/* 27 */
	gettext_noop(" СБОЙ   АРХИВА"),      	/* 28 */
	gettext_noop(" МАС.НЕ ВХ.В ПОЛЕ"),   	/* 29 */
	gettext_noop(" НЕТ КОНЦА БЦ.ИНФ"),   	/* 30 */
	gettext_noop(" ДЛ.МАССИВА>6200"),    	/* 31 */
	gettext_noop(" ДАЙ ТРАКТЫ"),         	/* 32 */
	gettext_noop(" ОБРАЩ.К НЕЗАК.МЛ"),   	/* 33 */
	gettext_noop(" ОШ.В ИНФ.СЛ.ЭКСТ"),   	/* 34 */
	gettext_noop(" ОБРАЩ.К НЕСУЩ.СП"),   	/* 35 */
	gettext_noop(" ЗАПРЕЩ.ЭКСТРАКОД"),   	/* 36 */
	gettext_noop(" ОШИБКА МЛ ЭВМ"),      	/* 37 */
	gettext_noop(" ИСТЕКЛО ВРЕМЯ"),      	/* 38 */
	gettext_noop(" ДАЙ МЕТРЫ АЦПУ!"),    	/* 39 */
	gettext_noop(" ИСТЕК.ВРЕМ.ПО ЭК"),   	/* 40 */
	gettext_noop(" ЧИСЛ.ВЫД.ПК>4096"),   	/* 41 */
	gettext_noop(" ОШИБКА МД ЭВМ"),      	/* 42 */
	gettext_noop(" НЕТ МЛ В ЭК.РЕС"),    	/* 43 */
	gettext_noop(" В ЭК.РЕС.ТРАК>32"),   	/* 44 */
	gettext_noop(" СНЯТА ПОЛЬЗОВАТ."),   	/* 45 */
	gettext_noop(" ДАЙ РАБ.В ЭК.РЕС"),   	/* 46 */
	gettext_noop(" ЗАПРЕЩ.НАПР.В ЭК"),   	/* 47 */
	gettext_noop(" ОШИБКА МБ ЭВМ"),      	/* 48 */
	gettext_noop(" НЕВЕРНЫЙ  ПАРОЛЬ"),   	/* 49 */
	gettext_noop(" ДАЙ ЗАПИСЬ НА МЛ"),   	/* 50 */
	gettext_noop(" УЗЕЛ"),               	/* 51 */
	gettext_noop(" АRСSIN(Х):|Х|>1"),    	/* 52 */
	gettext_noop(" КОРЕНЬ(Х): Х <0"),    	/* 53 */
	gettext_noop(" ЛОГАРИФМ(Х):Х<0"),    	/* 54 */
	gettext_noop(" ЕХР(Х): Х>44"),       	/* 55 */
	gettext_noop(" ЧИСЛО ЗОН>200"),      	/* 56 */
	gettext_noop(" ИНФ.СЛ.В ЧУЖ.ЛИС"),   	/* 57 */
	gettext_noop(" ЛИСТ В ЭК.ЧУЖОЙ"),    	/* 58 */
	gettext_noop(" МАССИВ НЕ СУЩ-Т."),   	/* 59 */
	gettext_noop(" СНЯЛАСЬ  ГЛАВНАЯ"),   	/* 60 */
	gettext_noop(" НЕТ МЕСТА В КАТА"),   	/* 61 */
	gettext_noop(" СНЯТА  ГЛАВНОЙ"),     	/* 62 */
	gettext_noop(" М220 НЕ ПЕРЕДАЛА"),   	/* 63 */
	gettext_noop(" ЧУЖАЯ ИНФОРМАЦИЯ"),   	/* 64 */
	gettext_noop(" СБО,ВВЕДИ ПО ИНС"),   	/* 65 */
	gettext_noop(" ЭКСТР.>ДОПУСТ."),     	/* 66 */
	gettext_noop(" ЗАДАЧА УПРЯТАНА"),    	/* 67 */
	gettext_noop(" НЕТ БОБИНЫ СВЯЗИ"),   	/* 68 */
	gettext_noop(" КЧ В РЕЖ.ТЕСТА"),     	/* 69 */
	gettext_noop(" НЕ ЗАКАЗАН ТЕРМ."),   	/* 70 */
	gettext_noop(" НЕКОР.ОБРАЩЕНИЕ"),    	/* 71 */
	gettext_noop(" ОБРЫВ СВЯЗИ"),        	/* 72 */
	gettext_noop(" ОШ.ЗП НА МЛ"),        	/* 73 */
	gettext_noop(" ПО СЧИТ."),           	/* 74 */
	gettext_noop(" ПО ЗАП."),            	/* 75 */
	gettext_noop(" КРА"),                	/* 76 */
	gettext_noop(" НЕТ ИНФ. ДЛЯ ОТЛ"),   	/* 77 */
	gettext_noop(" ЕNQ<=ОСТАН.=>DЕQ"),   	/* 78 */
	gettext_noop(" Err 79"),             	/* 79 */
	gettext_noop(" Err 80"),             	/* 80 */
	gettext_noop(" ОШ. УЧЕТА"),          	/* 81 */
};
