# Comparing sizes for build minsizerel

Files looked at (in build/minsizerel):
- FreeRTOS-Demo-0.elf.map
- FreeRTOS-Demo-1.elf.map
- FreeRTOS-Demo-2.elf.map
- FreeRTOS-Demo-3.elf.map
- FreeRTOS-Demo-4.elf.map
- FreeRTOS-Demo-5.elf.map
- FreeRTOS-Demo-6.elf.map
- FreeRTOS-Demo-7.elf.map

## Comparing FreeRTOS-Demo-0.elf.map to FreeRTOS-Demo-1.elf.map
Grew by 2.06KiB from section .text
- Grew by 4B from input startup.main
- Added 396B due to input [Ccf<...>::send(Channels, std::span<...>&)](# "Ccf<CcfConfig{256u, 256u, 255u}>::send(Channels, std::span<unsigned char, 4294967295u>&)")
- Added 56B due to input commsCcfStartTasks
- Added 40B due to input commsCcfTxAvailable
- Grew by 44B from input commsCcfRx
- Added 112B due to input [void CircularBuffer<...>::push_back<...>(unsigned char&&)](# "void CircularBuffer<unsigned char, 256u, 255u>::push_back<unsigned char>(unsigned char&&)")
- Added 66B due to input [std::__conditional<...>::type<...> std::ranges::__copy_or_move<...>(char const*, char const*, std::back_insert_iterator<...>)](# "std::__conditional<false>::type<std::ranges::in_out_result<char const*, std::back_insert_iterator<CircularBuffer<unsigned char, 256u, 255u> > >, std::ranges::in_out_result<char const*, std::back_insert_iterator<CircularBuffer<unsigned char, 256u, 255u> > > > std::ranges::__copy_or_move<false, char const*, char const*, std::back_insert_iterator<CircularBuffer<unsigned char, 256u, 255u> > >(char const*, char const*, std::back_insert_iterator<CircularBuffer<unsigned char, 256u, 255u> >)")
- Added 384B due to input commsCcfProcessTask(void*)
- Added 58B due to input [bool (anonymous namespace)::pack<...>(Cbor::Major, unsigned int, std::span<...>&)](# "bool (anonymous namespace)::pack<unsigned int>(Cbor::Major, unsigned int, std::span<unsigned char, 4294967295u>&)")
- Added 38B due to input [Cbor::packEmbedded(Cbor::Major, unsigned char, std::span<...>&)](# "Cbor::packEmbedded(Cbor::Major, unsigned char, std::span<unsigned char, 4294967295u>&)")
- Added 56B due to input [bool Cbor::encode<...>(Cbor::Major, unsigned char, std::span<...>&)](# "bool Cbor::encode<unsigned char>(Cbor::Major, unsigned char, std::span<unsigned char, 4294967295u>&)")
- Added 64B due to input [bool Cbor::encode<...>(Cbor::Major, unsigned short, std::span<...>&)](# "bool Cbor::encode<unsigned short>(Cbor::Major, unsigned short, std::span<unsigned char, 4294967295u>&)")
- Added 16B due to input [bool Cbor::encode<...>(Cbor::Major, unsigned int, std::span<...>&)](# "bool Cbor::encode<unsigned int>(Cbor::Major, unsigned int, std::span<unsigned char, 4294967295u>&)")
- Added 32B due to input Cobs::Encoder::findRunLength()
- Added 20B due to input Cobs::Encoder::operator*() const
- Added 76B due to input Cobs::Encoder::operator++()
- Added 18B due to input Cobs::Encoder::operator!=(Cobs::IteratorEnd) const
- Added 84B due to input vPortValidateInterruptPriority
- Added 172B due to input ulTaskGenericNotifyTake
- Added 268B due to input vTaskGenericNotifyGiveFromISR
- Added 50B due to input memmove
- Grew by 50B from input .rodata.IntDefaultHandler.str1.1
- Added 9B due to input .rodata.commsCcfStartTasks.str1.1
- Added 41B due to input .rodata.commsCcfProcessTask(void*) [clone .str1.1]


Grew by 2.09KiB from section .bss
- Added 4B due to input rxTask
- Added 88B due to input commsCcfTcb
- Added 2.00KiB due to input commsCcfStack

## Comparing FreeRTOS-Demo-1.elf.map to FreeRTOS-Demo-2.elf.map
Grew by 904B from section .text
- Added 48B due to input rpc::{lambda()#1}::_FUN()
- Added 66B due to input [Cbor::Cbor<...>::encode(std::basic_string_view<...>, std::span<...>&)](# "Cbor::Cbor<std::basic_string_view<char, std::char_traits<char> > >::encode(std::basic_string_view<char, std::char_traits<char> >, std::span<unsigned char, 4294967295u>&)")
- Added 104B due to input [Call<...>::schema(Cbor::Sequence<...>&) const](# "Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>::schema(Cbor::Sequence<(Cbor::Major)4>&) const")
- Added 404B due to input [bool Ccf<...>::poll<...>(Rpc<...> const&)](# "bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>> > const&)")
- Shrunk by 344B from input commsCcfProcessTask(void*)
- Added 104B due to input [Cbor::Cbor<...>::decode(std::span<...>&)](# "Cbor::Cbor<std::tuple<> >::decode(std::span<unsigned char, 4294967295u>&)")
- Added 126B due to input [Call<...>::call(std::span<...>&, std::span<...>&) const](# "Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>::call(std::span<unsigned char, 4294967295u>&, std::span<unsigned char, 4294967295u>&) const")
- Grew by 40B from input startup._GLOBAL__sub_I_ccf
- Added 84B due to input [bool (anonymous namespace)::pack<...>(Cbor::Major, unsigned long long, std::span<...>&)](# "bool (anonymous namespace)::pack<unsigned long long>(Cbor::Major, unsigned long long, std::span<unsigned char, 4294967295u>&)")
- Added 158B due to input [Cbor::unpack(std::span<...>&)](# "Cbor::unpack(std::span<unsigned char, 4294967295u>&)")
- Added 24B due to input [bool Cbor::encode<...>(Cbor::Major, unsigned long long, std::span<...>&)](# "bool Cbor::encode<unsigned long long>(Cbor::Major, unsigned long long, std::span<unsigned char, 4294967295u>&)")
- Added 16B due to input .text
- Grew by 28B from input .rodata.IntDefaultHandler.str1.1
- Added 11B due to input .rodata.rpc::{lambda()#1}::_FUN() [clone .str1.1]
- Added 41B due to input [.rodata.bool Ccf<...>::poll<...>(Rpc<...> const&) [clone .str1.1]](# ".rodata.bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>> > const&) [clone .str1.1]")
- Added 25B due to input .rodata._GLOBAL__sub_I_ccf.str1.1
- Added 21B due to input [.rodata.Type<...>::python](# ".rodata.Type<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long> >::python")
- Added 16B due to input [.rodata.vtable for Call<...>](# ".rodata.vtable for Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>")
- Removed 41B due to input .rodata.commsCcfProcessTask(void*) [clone .str1.1]


Grew by 24B from section .bss
- Added 24B due to input rpc

## Comparing FreeRTOS-Demo-2.elf.map to FreeRTOS-Demo-3.elf.map
Grew by 736B from section .text
- Added 4B due to input rpc::{lambda(unsigned long, unsigned long)#1}::_FUN(unsigned long, unsigned long)
- Added 24B due to input [Cbor::Sequence<...>::~Sequence()](# "Cbor::Sequence<(Cbor::Major)4>::~Sequence()")
- Added 420B due to input [bool Ccf<...>::poll<...>(Rpc<...> const&)](# "bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long> > const&)")
- Added 28B due to input [bool Cbor::Sequence<...>::encode<...>(std::basic_string_view<...>)](# "bool Cbor::Sequence<(Cbor::Major)4>::encode<std::basic_string_view<char, std::char_traits<char> > >(std::basic_string_view<char, std::char_traits<char> >)")
- Added 252B due to input [Call<...>::schema(Cbor::Sequence<...>&) const](# "Call<unsigned long, unsigned long, unsigned long>::schema(Cbor::Sequence<(Cbor::Major)4>&) const")
- Grew by 52B from input [Call<...>::schema(Cbor::Sequence<...>&) const](# "Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>::schema(Cbor::Sequence<(Cbor::Major)4>&) const")
- Added 60B due to input [Cbor::Cbor<...>::decode(std::span<...>&)](# "Cbor::Cbor<unsigned long>::decode(std::span<unsigned char, 4294967295u>&)")
- Added 160B due to input [Cbor::Cbor<...>::decode(std::span<...>&)](# "Cbor::Cbor<std::tuple<unsigned long, unsigned long> >::decode(std::span<unsigned char, 4294967295u>&)")
- Added 54B due to input [Call<...>::call(std::span<...>&, std::span<...>&) const](# "Call<unsigned long, unsigned long, unsigned long>::call(std::span<unsigned char, 4294967295u>&, std::span<unsigned char, 4294967295u>&) const")
- Grew by 52B from input startup._GLOBAL__sub_I_ccf
- Added 4B due to input [bool Cbor::encode<...>(Cbor::Major, unsigned long, std::span<...>&)](# "bool Cbor::encode<unsigned long>(Cbor::Major, unsigned long, std::span<unsigned char, 4294967295u>&)")
- Grew by 17B from input .rodata.IntDefaultHandler.str1.1
- Added 41B due to input [.rodata.bool Ccf<...>::poll<...>(Rpc<...> const&) [clone .str1.1]](# ".rodata.bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long> > const&) [clone .str1.1]")
- Grew by 19B from input .rodata._GLOBAL__sub_I_ccf.str1.1
- Added 4B due to input [.rodata.Type<...>::python](# ".rodata.Type<unsigned long>::python")
- Added 16B due to input [.rodata.vtable for Call<...>](# ".rodata.vtable for Call<unsigned long, unsigned long, unsigned long>")
- Removed 404B due to input [bool Ccf<...>::poll<...>(Rpc<...> const&)](# "bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>> > const&)")
- Removed 41B due to input [.rodata.bool Ccf<...>::poll<...>(Rpc<...> const&) [clone .str1.1]](# ".rodata.bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>> > const&) [clone .str1.1]")


Grew by 28B from section .bss
- Grew by 28B from input rpc

## Comparing FreeRTOS-Demo-3.elf.map to FreeRTOS-Demo-4.elf.map
Grew by 60B from section .text
- Added 4B due to input rpc::{lambda(unsigned long, unsigned long)#2}::_FUN(unsigned long, unsigned long)
- Added 420B due to input [bool Ccf<...>::poll<...>(Rpc<...> const&)](# "bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long> > const&)")
- Grew by 36B from input startup._GLOBAL__sub_I_ccf
- Grew by 15B from input .rodata.IntDefaultHandler.str1.1
- Added 41B due to input [.rodata.bool Ccf<...>::poll<...>(Rpc<...> const&) [clone .str1.1]](# ".rodata.bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long> > const&) [clone .str1.1]")
- Grew by 15B from input .rodata._GLOBAL__sub_I_ccf.str1.1
- Removed 420B due to input [bool Ccf<...>::poll<...>(Rpc<...> const&)](# "bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long> > const&)")
- Removed 41B due to input [.rodata.bool Ccf<...>::poll<...>(Rpc<...> const&) [clone .str1.1]](# ".rodata.bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long> > const&) [clone .str1.1]")


Grew by 28B from section .bss
- Grew by 28B from input rpc

## Comparing FreeRTOS-Demo-4.elf.map to FreeRTOS-Demo-5.elf.map
Grew by 308B from section .text
- Added 20B due to input rpc::{lambda()#2}::_FUN()
- Added 420B due to input [bool Ccf<...>::poll<...>(Rpc<...> const&)](# "bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >> > const&)")
- Added 152B due to input [Call<...>::schema(Cbor::Sequence<...>&) const](# "Call<std::basic_string_view<char, std::char_traits<char> >>::schema(Cbor::Sequence<(Cbor::Major)4>&) const")
- Added 48B due to input [Call<...>::call(std::span<...>&, std::span<...>&) const](# "Call<std::basic_string_view<char, std::char_traits<char> >>::call(std::span<unsigned char, 4294967295u>&, std::span<unsigned char, 4294967295u>&) const")
- Grew by 40B from input startup._GLOBAL__sub_I_ccf
- Grew by 24B from input .rodata.IntDefaultHandler.str1.1
- Added 41B due to input [.rodata.bool Ccf<...>::poll<...>(Rpc<...> const&) [clone .str1.1]](# ".rodata.bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >> > const&) [clone .str1.1]")
- Grew by 12B from input .rodata._GLOBAL__sub_I_ccf.str1.1
- Added 12B due to input .rodata.str1.1
- Added 16B due to input [.rodata.vtable for Call<...>](# ".rodata.vtable for Call<std::basic_string_view<char, std::char_traits<char> >>")
- Added 4B due to input [.rodata.Type<...>::python](# ".rodata.Type<std::basic_string_view<char, std::char_traits<char> > >::python")
- Added 8B due to input .rodata
- Removed 420B due to input [bool Ccf<...>::poll<...>(Rpc<...> const&)](# "bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long> > const&)")
- Removed 41B due to input [.rodata.bool Ccf<...>::poll<...>(Rpc<...> const&) [clone .str1.1]](# ".rodata.bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long> > const&) [clone .str1.1]")


Grew by 24B from section .bss
- Grew by 24B from input rpc

## Comparing FreeRTOS-Demo-5.elf.map to FreeRTOS-Demo-6.elf.map
Grew by 1.39KiB from section .text
- Added 6B due to input rpc::{lambda(unsigned int, unsigned int)#1}::_FUN(unsigned int, unsigned int)
- Added 30B due to input [rpc::{lambda(unsigned int, std::span<...>)#1}::_FUN(unsigned int, std::span<...>)](# "rpc::{lambda(unsigned int, std::span<unsigned char, 4294967295u>)#1}::_FUN(unsigned int, std::span<unsigned char, 4294967295u>)")
- Added 420B due to input [bool Ccf<...>::poll<...>(Rpc<...> const&)](# "bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >>, Call<std::span<unsigned char, 4294967295u>, unsigned int, unsigned int>, Call<void, unsigned int, std::span<unsigned char, 4294967295u> > > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >>, Call<std::span<unsigned char, 4294967295u>, unsigned int, unsigned int>, Call<void, unsigned int, std::span<unsigned char, 4294967295u> > > const&)")
- Added 264B due to input [Call<...>::schema(Cbor::Sequence<...>&) const](# "Call<void, unsigned int, std::span<unsigned char, 4294967295u> >::schema(Cbor::Sequence<(Cbor::Major)4>&) const")
- Added 260B due to input [Call<...>::schema(Cbor::Sequence<...>&) const](# "Call<std::span<unsigned char, 4294967295u>, unsigned int, unsigned int>::schema(Cbor::Sequence<(Cbor::Major)4>&) const")
- Added 60B due to input [Cbor::Cbor<...>::decode(std::span<...>&)](# "Cbor::Cbor<unsigned int>::decode(std::span<unsigned char, 4294967295u>&)")
- Added 160B due to input [Cbor::Cbor<...>::decode(std::span<...>&)](# "Cbor::Cbor<std::tuple<unsigned int, unsigned int> >::decode(std::span<unsigned char, 4294967295u>&)")
- Added 108B due to input [Call<...>::call(std::span<...>&, std::span<...>&) const](# "Call<std::span<unsigned char, 4294967295u>, unsigned int, unsigned int>::call(std::span<unsigned char, 4294967295u>&, std::span<unsigned char, 4294967295u>&) const")
- Added 36B due to input [std::optional<...> Cbor::Cbor<...>::transpose<...>(std::tuple<...>)](# "std::optional<std::tuple<unsigned int, std::span<unsigned char, 4294967295u> > > Cbor::Cbor<std::tuple<unsigned int, std::span<unsigned char, 4294967295u> > >::transpose<unsigned int, std::span<unsigned char, 4294967295u> >(std::tuple<std::optional<unsigned int>, std::optional<std::span<unsigned char, 4294967295u> > >)")
- Added 230B due to input [Cbor::Cbor<...>::decode(std::span<...>&)](# "Cbor::Cbor<std::tuple<unsigned int, std::span<unsigned char, 4294967295u> > >::decode(std::span<unsigned char, 4294967295u>&)")
- Added 52B due to input [Call<...>::call(std::span<...>&, std::span<...>&) const](# "Call<void, unsigned int, std::span<unsigned char, 4294967295u> >::call(std::span<unsigned char, 4294967295u>&, std::span<unsigned char, 4294967295u>&) const")
- Grew by 112B from input startup._GLOBAL__sub_I_ccf
- Grew by 59B from input .rodata.IntDefaultHandler.str1.1
- Added 41B due to input [.rodata.bool Ccf<...>::poll<...>(Rpc<...> const&) [clone .str1.1]](# ".rodata.bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >>, Call<std::span<unsigned char, 4294967295u>, unsigned int, unsigned int>, Call<void, unsigned int, std::span<unsigned char, 4294967295u> > > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >>, Call<std::span<unsigned char, 4294967295u>, unsigned int, unsigned int>, Call<void, unsigned int, std::span<unsigned char, 4294967295u> > > const&) [clone .str1.1]")
- Grew by 59B from input .rodata._GLOBAL__sub_I_ccf.str1.1
- Added 6B due to input [.rodata.Type<...>::python](# ".rodata.Type<std::span<unsigned char, 4294967295u> >::python")
- Added 4B due to input [.rodata.Type<...>::python](# ".rodata.Type<unsigned int>::python")
- Added 4B due to input [.rodata.Type<...>::python](# ".rodata.Type<void>::python")
- Added 16B due to input [.rodata.vtable for Call<...>](# ".rodata.vtable for Call<std::span<unsigned char, 4294967295u>, unsigned int, unsigned int>")
- Added 16B due to input [.rodata.vtable for Call<...>](# ".rodata.vtable for Call<void, unsigned int, std::span<unsigned char, 4294967295u> >")
- Removed 420B due to input [bool Ccf<...>::poll<...>(Rpc<...> const&)](# "bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >> > const&)")
- Removed 41B due to input [.rodata.bool Ccf<...>::poll<...>(Rpc<...> const&) [clone .str1.1]](# ".rodata.bool Ccf<CcfConfig{256u, 256u, 255u}>::poll<Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >> > >(Rpc<Call<std::tuple<std::basic_string_view<char, std::char_traits<char> >, std::basic_string_view<char, std::char_traits<char> >, unsigned long long>>, Call<unsigned long, unsigned long, unsigned long>, Call<unsigned long, unsigned long, unsigned long>, Call<std::basic_string_view<char, std::char_traits<char> >> > const&) [clone .str1.1]")


Grew by 56B from section .bss
- Grew by 56B from input rpc

## Comparing FreeRTOS-Demo-6.elf.map to FreeRTOS-Demo-7.elf.map
Grew by 4.09KiB from section .text
- Grew by 4B from input startup.main
- Added 6B due to input unlikely._isatty
- Added 48B due to input createLogTask
- Added 80B due to input [Ccf<...>::log(LogLevel, unsigned char, char const*, ...)](# "Ccf<CcfConfig{256u, 256u, 255u}>::log(LogLevel, unsigned char, char const*, ...)")
- Added 112B due to input logTask(void*)
- Added 336B due to input xQueueSemaphoreTake
- Added 76B due to input vTaskDelay
- Added 160B due to input xTaskPriorityInherit
- Added 172B due to input vTaskPriorityDisinheritAfterTimeout
- Added 20B due to input pvTaskIncrementMutexHeldCount
- Added 60B due to input __assert_func
- Added 36B due to input fprintf
- Added 92B due to input _vsnprintf_r
- Added 28B due to input vsnprintf
- Added 192B due to input __ssputs_r
- Added 496B due to input _svfprintf_r
- Added 42B due to input __sfputc_r
- Added 36B due to input __sfputs_r
- Added 552B due to input _vfprintf_r
- Added 228B due to input _printf_common
- Added 588B due to input _printf_i
- Added 114B due to input __smakebuf_r
- Added 124B due to input __swbuf_r
- Added 168B due to input __swsetup_r
- Added 32B due to input _isatty_r
- Added 28B due to input memchr
- Added 92B due to input _realloc_r
- Added 16B due to input _malloc_usable_size_r
- Grew by 244B from input .rodata.IntDefaultHandler.str1.1
- Added 4B due to input .rodata.createLogTask.str1.1
- Added 129B due to input .rodata.logTask(void*) [clone .str1.1]
- Added 61B due to input .rodata.__assert_func.str1.1
- Added 17B due to input .rodata._svfprintf_r.str1.1
- Added 17B due to input .rodata._vfprintf_r.str1.1
- Added 34B due to input .rodata._printf_i.str1.1


Grew by 1.09KiB from section .bss
- Added 88B due to input logTcb
- Added 1024B due to input logStack
