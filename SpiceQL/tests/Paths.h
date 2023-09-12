#include <vector>
#include <ghc/fs_std.hpp>


// paths for testing base / shared kernel queries
std::vector<std::string> base_paths = {
    "/isis_data/base/kernels/sclk/naif0001.tls",
    "/isis_data/base/kernels/sclk/naif0002.tls",
    "/isis_data/base/kernels/pck/pck00006.tpc"
};

// paths for testing messenger kernel queries
std::vector<std::string> mess_paths = {
    "/isis_data/messenger/kernels/ck/msgr_1234_v01.bc",
    "/isis_data/messenger/kernels/ck/msgr_1235_v01.bc",
    "/isis_data/messenger/kernels/ck/msgr_1235_v02.bc",
    "/isis_data/messenger/kernels/ck/msgr_1236_v03.bc",
    "/isis_data/messenger/kernels/sclk/messenger_0001.tsc",
    "/isis_data/messenger/kernels/sclk/messenger_0002.tsc",
    "/isis_data/messenger/kernels/spk/msgr_20040803_20150328_od360sc_0.bsp",
    "/isis_data/messenger/kernels/spk/msgr_20040803_20150328_od346sc_0.bsp",
    "/isis_data/messenger/kernels/fk/msgr_v001.tf",
    "/isis_data/messenger/kernels/fk/msgr_v002.tf",
    "/isis_data/messenger/kernels/iak/mdisAddendum001.ti",
    "/isis_data/messenger/kernels/iak/mdisAddendum002.ti",
    "/isis_data/messenger/kernels/ik/msgr_mdis_v001.ti",
    "/isis_data/messenger/kernels/ik/msgr_mdis_v002.ti",
    "/isis_data/messenger/kernels/pck/pck00010_msgr_v02.tpc",
    "/isis_data/messenger/kernels/pck/pck00010_msgr_v03.tpc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_sc332211_112233_sub_v1.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_sc332211_445566_sub_v1.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_sc332211_445566_sub_v2.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_sc665544_445566_sub_v2.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_sc3322_usgs_v1.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_sc3322_usgs_v2.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_sc1122_usgs_v1.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_sc0001_usgs_v3.bc",
    "/isis_data/messenger/kernels/ck/messenger_1122_v01.bc",
    "/isis_data/messenger/kernels/ck/messenger_3344_v01.bc",
    "/isis_data/messenger/kernels/ck/messenger_3344_v02.bc",
    "/isis_data/messenger/kernels/ck/messenger_5566_v03.bc",
    "/isis_data/messenger/kernels/tspk/de423s.bsp",
    "/isis_data/messenger/kernels/tspk/de405.bsp",
    "/isis_data/messenger/kernels/ck/msgr_mdis_gm332211_112233v1.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_gm332211_112233v2.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_gm012345_112233v1.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_gm332211_999999v1.bc",
    "/isis_data/messenger/kernels/ck/msgr_mdis_gm012345_999999v2.bc"
};

// paths for testing clementine kernel queries
std::vector<std::string> clem1_paths = {
    "/isis_data/clementine1/kernels/ck/clem_123.bck",
    "/isis_data/clementine1/kernels/ck/clem_124.bck",
    "/isis_data/clementine1/kernels/ck/clem_125.bck",
    "/isis_data/clementine1/kernels/ck/clem_126.bck",

    "/isis_data/clementine1/kernels/ck/clem_ulcn2005_6hr.bc",

    "/isis_data/clementine1/kernels/sclk/dspse001.tsc",
    "/isis_data/clementine1/kernels/sclk/dspse002.tsc",

    "/isis_data/clementine1/kernels/spk/clem_123_v01.bsp",
    "/isis_data/clementine1/kernels/spk/clem_124_v01.bsp",

    "/isis_data/clementine1/kernels/fk/clem_v01.tf",

    "/isis_data/clementine1/kernels/ik/clem_uvvis_beta_ik_v04.ti",

    "/isis_data/clementine1/kernels/iak/uvvisAddendum001.ti",
    "/isis_data/clementine1/kernels/iak/uvvisAddendum002.ti",
    "/isis_data/clementine1/kernels/iak/nirAddendum001.ti",
    "/isis_data/clementine1/kernels/iak/nirAddendum002.ti"
};


std::vector<std::string> galileo_paths = {
    "/isis_data/galileo/kernels/ck/ck90342a_plt.bc",
    "/isis_data/galileo/kernels/ck/ck90342b_plt.bc",
    "/isis_data/galileo/kernels/ck/ck90343a_plt.bc",
    "/isis_data/galileo/kernels/ck/CKmerge_type3.plt.bck",

    "/isis_data/galileo/kernels/ck/galssi_cal_med.bck",
    "/isis_data/galileo/kernels/ck/galssi_eur_usgs2020.bc",
    "/isis_data/galileo/kernels/ck/galssi_io_iau010806baa.bck",

    "/isis_data/galileo/kernels/sclk/mk00062b.tsc",

    "/isis_data/galileo/kernels/spk/s000131a.bsp",
    "/isis_data/galileo/kernels/spk/s000407a.bsp",

    "/isis_data/galileo/kernels/iak/ssiAddendum001.ti",

    "/isis_data/galileo/kernels/pck/pck00010_msgr_v23_europa2020.tpc"
};

// paths for apollo 17
std::vector<std::string> apollo17_paths = {
    "/isis_data/apollo17/kernels/ck/AS17_M_rev1.bc",
    "/isis_data/apollo17/kernels/ck/AS17_M_rev12.bc",
    "/isis_data/apollo17/kernels/ck/AS17_M_REV1234_v2.bc",
    "/isis_data/apollo17/kernels/ck/AS17_M_REV45_v2.bc",

    "/isis_data/apollo17/kernels/sclk/apollo17.5522.tsc",

    "/isis_data/apollo17/kernels/fk/apollo17.0001.tf",
    "/isis_data/apollo17/kernels/fk/apollo17_v2.8968.tf",

    "/isis_data/apollo17/kernels/spk/AS17_M_REV7.bsp",
    "/isis_data/apollo17/kernels/spk/AS17_M_REV13.bsp",
    "/isis_data/apollo17/kernels/spk/AS17_M_REV8799_v2.bsp",
    "/isis_data/apollo17/kernels/spk/AS17_M_REV88_v2.bsp",

    "/isis_data/apollo17/kernels/iak/apollo17MetricAddendum333.ti",
    "/isis_data/apollo17/kernels/iak/apolloPanAddendum986.ti",

    "/isis_data/apollo17/kernels/ik/apollo17_metric.1234.ti",
    "/isis_data/apollo17/kernels/ik/apollo17_metric_v2.1234.ti",
    "/isis_data/apollo17/kernels/ik/apollo17_panoramic.6214.ti"
    };
    
std::vector<std::string> cassini_paths = {
    "isis_data/cassini/ck/123456_123456r.bc",
    "isis_data/cassini/ck/12345_12345r.bc",
    "isis_data/cassini/ck/Enceladus_CISS_2019Shape_camera.bc",
    "isis_data/cassini/ck/99213_99243cb_ISS.bc/",
    "isis_data/cassini/ck/04444_55555c1_ISS.bc",

    "isis_data/cassini/fk/cas_v21_usgs.tf",
    "isis_data/cassini/fk/cas_v21.tf",

    "isis_data/cassini/iak/vimsAddendum78.ti",
    "isis_data/cassini/iak/IssNAAddendum777.ti",
    "isis_data/cassini/iak/IssWAAddendum123.ti",

    "isis_data/cassini/pck/naif0012.tls",
    "isis_data/cassini/pck/pck12345.tpc",
    "isis_data/cassini/pck/cpck15Dec2017_2019Shape.tpc",

    "isis_data/cassini/sclk/cas87901.tsc",

    "isis_data/cassini/spk/010420R_SCPSE_EP1_JP83.bsp",
    "isis_data/cassini/spk/990135R_SCPSE_78992_87123.bsp",
    "isis_data/cassini/spk/cpck30Sep2004_jupiter.tpc"
};

std::vector<std::string> lro_paths = {
    "/isis_data/lro/kernels/tspk/de421.bsp",
    "/isis_data/lro/kernels/tspk/moon_pa_de421_1900-2050.bpc",

    "/isis_data/lro/kernels/fk/lro_frames_2012255_v02.tf",
    "/isis_data/lro/kernels/fk/lro_frames_2014049_v01.tf",

    "/isis_data/lro/kernels/ik/lro_lroc_v17.ti",
    "/isis_data/lro/kernels/ik/lro_lroc_v18.ti",

    "/isis_data/lro/kernels/iak/lro_instrumentAddendum_v03.ti",
    "/isis_data/lro/kernels/iak/lro_instrumentAddendum_v04.ti",

    "/isis_data/lro/kernels/pck/moon_080317.tf",
    "/isis_data/lro/kernels/pck/moon_assoc_me.tf",

    "/isis_data/lro/kernels/ck/moc42_2021272_2021273_v01.bc",
    "/isis_data/lro/kernels/ck/moc42_2021273_2021274_v01.bc",
    "/isis_data/lro/kernels/ck/moc42_2021274_2021275_v01.bc",
    "/isis_data/lro/kernels/ck/moc42_2021275_2021276_v01.bc",
    "/isis_data/lro/kernels/ck/moc42r_2021120_2021152_v01.bc",
    "/isis_data/lro/kernels/ck/moc42r_2021151_2021182_v01.bc",
    "/isis_data/lro/kernels/ck/moc42r_2021181_2021213_v01.bc",
    "/isis_data/lro/kernels/ck/moc42r_2021212_2021244_v01.bc",
    "/isis_data/lro/kernels/ck/lrolc_2021120_2021152_v01.bc",
    "/isis_data/lro/kernels/ck/lrolc_2021151_2021182_v01.bc",
    "/isis_data/lro/kernels/ck/lrolc_2021181_2021213_v01.bc",
    "/isis_data/lro/kernels/ck/lrolc_2021212_2021244_v01.bc",
    "/isis_data/lro/kernels/ck/soc31_2021273_2021274_v01.bc",
    "/isis_data/lro/kernels/ck/soc31_2021274_2021275_v01.bc",
    "/isis_data/lro/kernels/ck/soc31_2021275_2021276_v01.bc",
    "/isis_data/lro/kernels/ck/soc31_2021276_2021277_v01.bc",

    "/isis_data/lro/kernels/spk/fdf29_2021273_2021274_b01.bsp",
    "/isis_data/lro/kernels/spk/fdf29_2021274_2021275_n01.bsp",
    "/isis_data/lro/kernels/spk/fdf29_2021275_2021276_n01.bsp",
    "/isis_data/lro/kernels/spk/fdf29_2021276_2021277_n01.bsp",
    "/isis_data/lro/kernels/spk/fdf29r_2021121_2021152_v01.bsp",
    "/isis_data/lro/kernels/spk/fdf29r_2021152_2021182_v01.bsp",
    "/isis_data/lro/kernels/spk/fdf29r_2021182_2021213_v01.bsp",
    "/isis_data/lro/kernels/spk/fdf29r_2021213_2021244_v01.bsp",
    "/isis_data/lro/kernels/spk/LRO_CO_201308_GRGM660PRIMAT270.bsp",
    "/isis_data/lro/kernels/spk/LRO_CO_201311_GRGM900C_L600.BSP",
    "/isis_data/lro/kernels/spk/LRO_ES_08_201308_GRGM660PRIMAT270.bsp",
    "/isis_data/lro/kernels/spk/LRO_ES_09_201308_GRGM660PRIMAT270.bsp",
    "/isis_data/lro/kernels/spk/LRO_NO_12_201308_GRGM660PRIMAT270.bsp",
    "/isis_data/lro/kernels/spk/LRO_NO_13_201308_GRGM660PRIMAT270.bsp",
    "/isis_data/lro/kernels/spk/LRO_SM_25_201311_GRGM900C_L600.BSP",
    "/isis_data/lro/kernels/spk/LRO_SM_26_201311_GRGM900C_L600.BSP",
    "/isis_data/lro/kernels/spk/LRO_ES_85_201910_GRGM900C_L600.BSP",
    "/isis_data/lro/kernels/spk/LRO_ES_86_201910_GRGM900C_L600.BSP",
    "/isis_data/lro/kernels/spk/LRO_NO_12_201311_GRGM900C_L600.BSP",
    "/isis_data/lro/kernels/spk/LRO_NO_13_201311_GRGM900C_L600.BSP",
    "/isis_data/lro/kernels/spk/LRO_SM_25_201308_GRGM660PRIMAT270.bsp",
    "/isis_data/lro/kernels/spk/LRO_SM_26_201308_GRGM660PRIMAT270.bsp",

    "/isis_data/lro/kernels/sclk/lro_clkcor_2021327_v00.tsc",
    "/isis_data/lro/kernels/sclk/lro_clkcor_2021287_v00.tsc"
};

// paths for testing apollo16 kernel queries
std::vector<std::string> apollo16_paths = {
    "/isis_data/apollo16/kernels/sclk/apollo16.0002.tsc",

    "/isis_data/apollo16/kernels/ck/AS16_M_REV1.bc",
    "/isis_data/apollo16/kernels/ck/AS16_M_REV02.bc",
    "/isis_data/apollo16/kernels/ck/AS16_M_REV01-23_v2.bc",
    "/isis_data/apollo16/kernels/ck/AS16_M_REV22_v2.bc",

    "/isis_data/apollo16/kernels/spk/AS16_M_REV2.bsp",
    "/isis_data/apollo16/kernels/spk/AS16_M_REV90.bsp",
    "/isis_data/apollo16/kernels/spk/AS16_M_REV7921_v2.bsp",
    "/isis_data/apollo16/kernels/spk/AS16_M_REV31_v2.bsp",

    "/isis_data/apollo16/kernels/fk/apollo16.0001.tf",
    "/isis_data/apollo16/kernels/fk/apollo16_v2.1234.tf",

    "/isis_data/apollo16/kernels/ik/apollo16_metric.1234.ti",
    "/isis_data/apollo16/kernels/ik/apollo16_metric_v2.2411.ti",
    "/isis_data/apollo16/kernels/ik/apollo16_panoramic.1234.ti"
    "/isis_data/apollo16/kernels/iak/apolloPanAddendum701.ti",

    "/isis_data/apollo16/kernels/iak/apollo16MetricAddendum123.ti"
};

std::vector<std::string> juno_paths = {
    "/isis_data/juno/kernels/ck/juno_sc_rec_110915_110917_v03.bc",
    "/isis_data/juno/kernels/ck/juno_sc_rec_110918_110924_v03.bc",
    "/isis_data/juno/kernels/ck/juno_sc_rec_110925_111001_v03.bc",
    "/isis_data/juno/kernels/ck/juno_sc_rec_111002_111008_v03.bc",

    "/isis_data/juno/kernels/spk/juno_rec_110805_111026_120302.bsp",
    "/isis_data/juno/kernels/spk/juno_rec_111026_120308_120726.bsp",

    "/isis_data/juno/kernels/sclk/jno_sclkscet_00058.tsc",

    "/isis_data/juno/kernels/tspk/de436s.bsp",
    "/isis_data/juno/kernels/tspk/de438s.bsp",
    "/isis_data/juno/kernels/tspk/juno_struct_v04.bsp",

    "/isis_data/juno/kernels/fk/juno_v12.tf",
    "/isis_data/juno/kernels/ik/juno_junocam_v03.ti",
    "/isis_data/juno/kernels/iak/junoAddendum005.ti",

    "/isis_data/juno/kernels/pck/pck00010.tpc"
};

// paths for viking1
std::vector<std::string> viking1_paths = {
    "/isis_data/viking1/kernels/ck/vo1_sedr.bc",
    "/isis_data/viking1/kernels/ck/vo1_mdim2.0_rand.bc",
    "/isis_data/viking1/kernels/fk/vol1_v12.tf",
    "/isis_data/viking1/kernels/iak/vikingAddendum123.ti",
    "/isis_data/viking1/kernels/sclk/vo1_fict.tsc",
    "/isis_data/viking1/kernels/sclk/vo1_fsc.tsc",
    "isis_data/viking1/kernels/spk/v01_sedr.bsp"
};

// paths for viking2
std::vector<std::string> viking2_paths = {
    "/isis_data/viking2/kernels/ck/vo2_sedr_kck2.bc",
    "/isis_data/viking2/kernels/ck/vo2bType3Csmithed.bc",
    "/isis_data/viking2/kernels/fk/vo2_v31.tf",
    "/isis_data/viking2/kernels/iak/vikingAddendum123.ti",
    "/isis_data/viking2/kernels/sclk/vo2_fict.tsc",
    "/isis_data/viking2/kernels/sclk/vo2_fsc.tsc",
    "isis_data/viking2/kernels/spk/viking2a.bsp",
    "isis_data/viking2/kernels/spk/vo2_recon.bsp",
    "isis_data/viking2/kernels/spk/vo2_sedr.bsp",
};

