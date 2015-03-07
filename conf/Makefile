CONFIG_DIR=/etc/tina
SERVICE_DIR=/etc/init.d
install:
	$(shell [ -d ${CONFIG_DIR} ] || mkdir -p ${CONFIG_DIR})
	cp tina.conf $(CONFIG_DIR)
	cp -r error $(CONFIG_DIR)
	cp tinad $(SERVICE_DIR)
