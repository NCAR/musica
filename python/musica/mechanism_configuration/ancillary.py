from .. import backend

_backend = backend.get_backend()
Version = _backend._mechanism_configuration._Version
Parser = _backend._mechanism_configuration._Parser
ReactionType = _backend._mechanism_configuration._ReactionType
