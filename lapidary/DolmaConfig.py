class DolmaConfig:
    def __init__(self, name=None, stt_threat_model=None, mode=None, is_smt=0):
        self.name = name
        self.stt_threat_model = stt_threat_model
        self.mode = mode
        self.is_smt = is_smt
    @staticmethod
    def before_init(system):
        pass

    @staticmethod
    def after_warmup():
        pass

def baseline_no():
    return DolmaConfig(
        name='baseline_no',
        mode=0,
    )

def baseline_smt():
    return DolmaConfig(
        name='baseline_smt',
        mode=0,
        is_smt=1,
    )

def dolma_default_no():
    return DolmaConfig(
        name='dolma_default_no',
        mode=1,
        is_smt=0,
    )

def dolma_default_mem_only_no():
    return DolmaConfig(
        name='dolma_default_mem_only_no',
        mode=3,
        is_smt=0,
    )

def dolma_conservative_mem_only_no():
    return DolmaConfig(
        name='dolma_conservative_mem_only_no',
        mode=4,
        is_smt=0,
    )

def dolma_default_smt_mem_only():
    return DolmaConfig(
        name='dolma_default_smt_mem_only',
        mode=3,
        is_smt=1,
    )

def dolma_conservative_smt_mem_only():
    return DolmaConfig(
        name='dolma_conservative_smt_mem_only',
        mode=4,
        is_smt=1,
    )

def dolma_default_smt():
    return DolmaConfig(
        name='dolma_default_smt',
        mode=1,
        is_smt=1,
    )

def dolma_default_mem_only_smt():
    return DolmaConfig(
        name='dolma_default_mem_only_smt',
        mode=3,
        is_smt=1,
    )

def dolma_conservative_no():
    return DolmaConfig(
        name='dolma_conservative_no',
        mode=2,
        is_smt=0,
    )

def dolma_conservative_smt():
    return DolmaConfig(
        name='dolma_conservative_smt',
        mode=2,
        is_smt=1,
    )

def dolma_conservative_mem_only_smt():
    return DolmaConfig(
        name='dolma_conservative_mem_only_smt',
        mode=4,
        is_smt=1,
    )

def stt_default_no():
    return DolmaConfig(
        name='stt_default_no',
        mode=1,
        is_smt=0,
    )

def stt_default_mem_only_no():
    return DolmaConfig(
        name='stt_default_mem_only_no',
        mode=3,
        is_smt=0,
    )

def stt_conservative_mem_only_no():
    return DolmaConfig(
        name='stt_conservative_mem_only_no',
        mode=4,
        is_smt=0,
    )

def stt_default_smt_mem_only():
    return DolmaConfig(
        name='stt_default_smt_mem_only',
        mode=3,
        is_smt=1,
    )

def stt_conservative_smt_mem_only():
    return DolmaConfig(
        name='stt_conservative_smt_mem_only',
        mode=4,
        is_smt=1,
    )

def stt_default_smt():
    return DolmaConfig(
        name='stt_default_smt',
        mode=1,
        is_smt=1,
    )

def stt_default_mem_only_smt():
    return DolmaConfig(
        name='stt_default_mem_only_smt',
        mode=3,
        is_smt=1,
    )

def stt_conservative_no():
    return DolmaConfig(
        name='stt_conservative_no',
        mode=2,
        is_smt=0,
    )

def stt_conservative_smt():
    return DolmaConfig(
        name='stt_conservative_smt',
        mode=2,
        is_smt=1,
    )

def stt_conservative_mem_only_smt():
    return DolmaConfig(
        name='stt_conservative_mem_only_smt',
        mode=4,
        is_smt=1,
    )

def generate_configs(config_group):
    if config_group == 'baseline_no':
        return [baseline_no()]
    elif config_group == 'nda_baseline':
        return [nda_baseline()]
    elif config_group == 'dolma_default_no':
        return [dolma_default_no()]
    elif config_group == 'dolma_default_mem_only_no':
        return [dolma_default_mem_only_no()]
    elif config_group == 'dolma_conservative_no':
        return [dolma_conservative_no()]
    elif config_group == 'dolma_conservative_mem_only_no':
        return [dolma_conservative_mem_only_no()]
    elif config_group == 'stt_default_no':
        return [stt_default_no()]
    elif config_group == 'stt_default_mem_only_no':
        return [stt_default_mem_only_no()]
    elif config_group == 'stt_conservative_no':
        return [stt_conservative_no()]
    elif config_group == 'stt_conservative_mem_only_no':
        return [stt_conservative_mem_only_no()]
    elif config_group == 'baseline_smt':
        return [baseline_smt()]
    elif config_group == 'dolma_default_smt':
        return [dolma_default_smt()]
    elif config_group == 'dolma_default_mem_only_smt':
        return [dolma_default_mem_only_smt()]
    elif config_group == 'dolma_conservative_smt':
        return [dolma_conservative_smt()]
    elif config_group == 'dolma_conservative_mem_only_smt':
        return [dolma_conservative_mem_only_smt()]
    elif config_group == 'stt_default_smt':
        return [stt_default_smt()]
    elif config_group == 'stt_default_mem_only_smt':
        return [stt_default_mem_only_smt()]
    elif config_group == 'stt_conservative_smt':
        return [stt_conservative_smt()]
    elif config_group == 'stt_conservative_mem_only_smt':
        return [stt_conservative_mem_only_smt()]
    elif config_group == 'single':
        return [dolma_default_no(), stt_default_no(), dolma_default_mem_only_no(), stt_default_mem_only_no(), dolma_conservative_no(), stt_conservative_no(), dolma_conservative_mem_only_no(), stt_conservative_mem_only_no(), baseline_no()]
    elif config_group == 'smt':
        return [dolma_default_smt(), stt_default_smt(), dolma_default_mem_only_smt(), stt_default_mem_only_smt(), dolma_conservative_smt(), stt_conservative_smt(), dolma_conservative_mem_only_smt(), stt_conservative_mem_only_smt(), baseline_smt()]
    else:
        assert False, "Need a valid config group"
