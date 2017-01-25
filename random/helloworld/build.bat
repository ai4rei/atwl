@IF NOT EXIST helloworld.tlb MIDL @helloworld.midl
@RC /Fohelloworld.res helloworld.rc
@CL @helloworld.build