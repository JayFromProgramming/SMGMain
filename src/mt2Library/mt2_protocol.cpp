//
// Created by Jay on 4/20/2022.
//

#include "mt2_protocol.h"

namespace mt2 {


// Respawn health lookup table
    short heath_lookup(respawn_health health) {
        switch (health) {
            case HP_1:
                return 1;
            case HP_2:
                return 2;
            case HP_3:
                return 3;
            case HP_4:
                return 4;
            case HP_5:
                return 5;
            case HP_6:
                return 6;
            case HP_7:
                return 7;
            case HP_8:
                return 8;
            case HP_9:
                return 9;
            case HP_10:
                return 10;
            case HP_11:
                return 11;
            case HP_12:
                return 12;
            case HP_13:
                return 13;
            case HP_14:
                return 14;
            case HP_15:
                return 15;
            case HP_16:
                return 16;
            case HP_17:
                return 17;
            case HP_18:
                return 18;
            case HP_19:
                return 19;
            case HP_20:
                return 20;
            case HP_25:
                return 25;
            case HP_30:
                return 30;
            case HP_35:
                return 35;
            case HP_40:
                return 40;
            case HP_45:
                return 45;
            case HP_50:
                return 50;
            case HP_55:
                return 55;
            case HP_60:
                return 60;
            case HP_65:
                return 65;
            case HP_70:
                return 70;
            case HP_75:
                return 75;
            case HP_80:
                return 80;
            case HP_85:
                return 85;
            case HP_90:
                return 90;
            case HP_95:
                return 95;
            case HP_100:
                return 100;
            case HP_105:
                return 105;
            case HP_110:
                return 110;
            case HP_115:
                return 115;
            case HP_120:
                return 120;
            case HP_125:
                return 125;
            case HP_130:
                return 130;
            case HP_135:
                return 135;
            case HP_140:
                return 140;
            case HP_145:
                return 145;
            case HP_150:
                return 150;
            case HP_155:
                return 155;
            case HP_160:
                return 160;
            case HP_165:
                return 165;
            case HP_170:
                return 170;
            case HP_175:
                return 175;
            case HP_180:
                return 180;
            case HP_185:
                return 185;
            case HP_190:
                return 190;
            case HP_195:
                return 195;
            case HP_200:
                return 200;
            case HP_250:
                return 250;
            case HP_300:
                return 300;
            case HP_350:
                return 350;
            case HP_400:
                return 400;
            case HP_450:
                return 450;
            case HP_500:
                return 500;
            case HP_550:
                return 550;
            case HP_600:
                return 600;
            case HP_650:
                return 650;
            case HP_700:
                return 700;
            case HP_750:
                return 750;
            case HP_800:
                return 800;
            case HP_850:
                return 850;
            case HP_900:
                return 900;
            case HP_950:
                return 950;
            case HP_999:
                return 999;
            default:
                return 0;
        }
    }

    short hit_delay_to_micros(hit_delays delay_setting){
        switch (delay_setting){
            case NO_DELAY:
                return 0;
            case ONE_QUARTER_SECOND:
                return 250;
            case HALF_SECOND:
                return 500;
            case THREE_QUARTER_SECOND:
                return 750;
            case ONE_SECOND:
                return 1000;
            case TWO_SECONDS:
                return 2000;
            case THREE_SECONDS:
                return 3000;
            case FOUR_SECONDS:
                return 4000;
            case FIVE_SECONDS:
                return 5000;
            case SIX_SECONDS:
                return 6000;
            case SEVEN_SECONDS:
                return 7000;
            case EIGHT_SECONDS:
                return 8000;
            case NINE_SECONDS:
                return 9000;
            case TEN_SECONDS:
                return 10000;
            case ELEVEN_SECONDS:
                return 11000;
            case TWELVE_SECONDS:
                return 12000;
            case THIRTEEN_SECONDS:
                return 13000;
            case FOURTEEN_SECONDS:
                return 14000;
            case FIFTEEN_SECONDS:
                return 15000;
            case SIXTEEN_SECONDS:
                return 16000;
            case SEVENTEEN_SECONDS:
                return 17000;
            case EIGHTEEN_SECONDS:
                return 18000;
            case NINETEEN_SECONDS:
                return 19000;
            case TWENTY_SECONDS:
                return 20000;
            default:
                return 0;
        }
    }

    short damage_table_lookup(damage_table damage){
        switch (damage) {

            case DAMAGE_1:
                return 1;
            case DAMAGE_2:
                return 2;
            case DAMAGE_4:
                return 4;
            case DAMAGE_5:
                return 5;
            case DAMAGE_7:
                return 7;
            case DAMAGE_10:
                return 10;
            case DAMAGE_15:
                return 15;
            case DAMAGE_17:
                return 17;
            case DAMAGE_20:
                return 20;
            case DAMAGE_25:
                return 25;
            case DAMAGE_30:
                return 30;
            case DAMAGE_35:
                return 35;
            case DAMAGE_40:
                return 40;
            case DAMAGE_50:
                return 50;
            case DAMAGE_75:
                return 75;
            case DAMAGE_100:
                return 100;
        }
    };
}