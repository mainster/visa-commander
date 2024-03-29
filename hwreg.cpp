#include <QtCore/QtGlobal>
#include <QString>
#include <QVector>

#include "hwreg.h"
#include "utype.h"


/** If the header is included multiple times we have to outsource the
 * "description" define into implementation file to avoid "... multiple
 * definitions..." compiler errors
 */
const quint8 HwReg::address = 0; 
const QString HwReg::description = 
      "Polymorphic superclass";


Reg_0609 * Reg_0609::pInst = 0x00;
const quint8 Reg_0609::address = 0x06;
const QString Reg_0609::description = 
      "h06..h09   act_RAM_addr: Actual RAM address (read scope samples) | "
      "ADDR_after_trig: RAM address after trigger event (Scope_state changes to 3)";

Reg_0a0d * Reg_0a0d::pInst = 0x00;
const quint8 Reg_0a0d::address = 0x0a;
const QString Reg_0a0d::description = 
      "0a0d   Trig_event_dist | Sampling ticks between two trigger events";

Reg_1017 * Reg_1017::pInst = 0x00;
const quint8 Reg_1017::address = 0x10;
const QString Reg_1017::description = 
      "1017   ADC value Pwr: Vplus, Vminus, Iplus, Iminus";

Reg_181f * Reg_181f::pInst = 0x00;
const quint8 Reg_181f::address = 0x18;
const QString Reg_181f::description =
      "181f   ADC value Vin | ADC value DVM";

Reg_3037 * Reg_3037::pInst = 0x00;
const quint8 Reg_3037::address = 0x30;
const QString Reg_3037::description =
      "3037   FuGen ampl, offset, duty, ampl rng, wafeform, noisetyp";

Reg_383b * Reg_383b::pInst = 0x00;
const quint8 Reg_383b::address = 0x38;
const QString Reg_383b::description =
      "383b   FuGen DDS, frequency of output signal";

/** BIT STAGING NOT VERIFIED AT THIS POINT */
Reg_6063 * Reg_6063::pInst = 0x00;
const quint8 Reg_6063::address = 0x60;
const QString Reg_6063::description =
      "6063   Scope-XA, bits for free-run, trigger, start/stop - RAM_read_Addr";

/** BIT STAGING NOT VERIFIED AT THIS POINT */
Reg_7073 * Reg_7073::pInst = 0x00;
const quint8 Reg_7073::address = 0x70;
const QString Reg_7073::description =
      "7073   Ch-1, Gnd-position, ACDC-coupling, input filter config";

/** BIT STAGING NOT VERIFIED AT THIS POINT */
Reg_7477 * Reg_7477::pInst = 0x00;
const quint8 Reg_7477::address = 0x74;
const QString Reg_7477::description =
      "7477   Ch-2, Gnd-position, ACDC-coupling, input filter config";

Reg_8087 * Reg_8087::pInst = 0x00;
const quint8 Reg_8087::address = 0x80;
const QString Reg_8087::description =
      "h80..h87   PWR_set ||  64-Bit register ||  BIT-adressierbar";

Reg_8a8b * Reg_8a8b::pInst = 0x00;
const quint8 Reg_8a8b::address = 0x8a;
const QString Reg_8a8b::description =
      "h8a..h8b   DMM Range || 16-Bit register ||  BIT-adressierbar";

Reg_bebf * Reg_bebf::pInst = 0x00;
const quint8 Reg_bebf::address = 0xbe;
const QString Reg_bebf::description =
      "hbe..hbf   LED  ||  16-Bit register ||  BIT-adressierbar";

/** Methode which returns a vector type of physical metrics for ADC values */
//Utype::metrics_t Reg_1017::getPhys() {
//   return metrics;
//}

//   Utype::metrics_t Reg_1017::getCalc(Utype::calDat_t calDat) {
//Utype::metrics_t Reg_1017::getCalc() {
//   Utype::measure_t meas;
//   Calc calc;

//   /** Reg_1017: calibrated hw data: Hc(14) */
//   meas.tol    = calc.cCalib[14].H;
//   meas.value  = h1017_adcPwr. calc.cCalib[14].H

//   return metrics;
//   Utype::measure_t *met1; // = new Utype::measure_t();
//   return met1;
//}

//Utype::metrics_t Reg_181f::operator=(const Utype::metrics_t& right) {
//   Utype::metrics_t res;
//   res = right;
//}


//QVector<double> Reg_181f::getCalc() {
////   Utype::measure_t *meas = new Utype::measure_t();
//   Calc calc;
////   QString * mUnit = new QString("V dc");

//   double meas;
//   meas  = (( h181f_dvm.ADC_Vbias - h181f_dvm.ADC_gnd )
//            * calc.cCalib[14].H);
//   QVector<double> metrics;
   
//   return metrics;
//}
