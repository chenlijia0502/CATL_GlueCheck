from library.parametersetting.BaseParameterSetting import KxBaseParameterSetting
import imc_msg


class ChipParameterSetting(KxBaseParameterSetting):
    def __init__(self, hparent, dict_config):
        super(ChipParameterSetting, self).__init__(hparent, dict_config)

    def recmsg(self, n_stationid, n_msgtype, tuple_data):
        # super(ChipParameterSetting, self).recmsg(n_stationid, n_msgtype, tuple_data)
        if n_msgtype == imc_msg.MSG_LEARN_ONE_COMPLETED:
            self.str2paramitemfun(n_stationid, 1, "setlearnstatus", False)
