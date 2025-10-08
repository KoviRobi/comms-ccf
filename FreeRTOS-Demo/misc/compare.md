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
Increase 2.06KiB due to output .text

- Add 396B due to input <details style="display:inline-block;"><summary><a>.text.Ccf&lt;...&gt;::send(Channels, std::span&lt;...&gt;&amp;)</a></summary>.text.Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::send(Channels, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 384B due to input .text.commsCcfProcessTask(void*)
- Add 268B due to input .text.vTaskGenericNotifyGiveFromISR
- Add 172B due to input .text.ulTaskGenericNotifyTake
- Add 112B due to input <details style="display:inline-block;"><summary><a>.text.void CircularBuffer&lt;...&gt;::push_back&lt;...&gt;(unsigned char&amp;&amp;)</a></summary>.text.void CircularBuffer&lt;unsigned char, 256u, 255u&gt;::push_back&lt;unsigned char&gt;(unsigned char&amp;&amp;)</details>
- Add 84B due to input .text.vPortValidateInterruptPriority
- Add 76B due to input .text.Cobs::Encoder::operator++()
- Add 68B due to input <details style="display:inline-block;"><summary><a>.text.std::__conditional&lt;...&gt;::type&lt;...&gt; std::ranges::__copy_or_move&lt;...&gt;(char const*, char const*, std::back_insert_iterator&lt;...&gt;)</a></summary>.text.std::__conditional&lt;false&gt;::type&lt;std::ranges::in_out_result&lt;char const*, std::back_insert_iterator&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt; &gt; &gt;, std::ranges::in_out_result&lt;char const*, std::back_insert_iterator&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt; &gt; &gt; &gt; std::ranges::__copy_or_move&lt;false, char const*, char const*, std::back_insert_iterator&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt; &gt; &gt;(char const*, char const*, std::back_insert_iterator&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt; &gt;)</details>
- Add 64B due to input <details style="display:inline-block;"><summary><a>.text.bool Cbor::encode&lt;...&gt;(Cbor::Major, unsigned short, std::span&lt;...&gt;&amp;)</a></summary>.text.bool Cbor::encode&lt;unsigned short&gt;(Cbor::Major, unsigned short, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 58B due to input <details style="display:inline-block;"><summary><a>.text.bool (anonymous namespace)::pack&lt;...&gt;(Cbor::Major, unsigned int, std::span&lt;...&gt;&amp;)</a></summary>.text.bool (anonymous namespace)::pack&lt;unsigned int&gt;(Cbor::Major, unsigned int, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 56B due to input .text.commsCcfStartTasks
- Add 56B due to input <details style="display:inline-block;"><summary><a>.text.bool Cbor::encode&lt;...&gt;(Cbor::Major, unsigned char, std::span&lt;...&gt;&amp;)</a></summary>.text.bool Cbor::encode&lt;unsigned char&gt;(Cbor::Major, unsigned char, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 50B due to input .text.memmove
- Increase 50B due to input .rodata.IntDefaultHandler.str1.1
- Increase 44B due to input .text.commsCcfRx
- Add 41B due to input .rodata.commsCcfProcessTask(void*) [clone .str1.1]
- Add 40B due to input .text.commsCcfTxAvailable
- Add 38B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::packEmbedded(Cbor::Major, unsigned char, std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::packEmbedded(Cbor::Major, unsigned char, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 32B due to input .text.Cobs::Encoder::findRunLength()
- Add 20B due to input .text.Cobs::Encoder::operator*() const
- Add 18B due to input .text.Cobs::Encoder::operator!=(Cobs::IteratorEnd) const
- Add 16B due to input <details style="display:inline-block;"><summary><a>.text.bool Cbor::encode&lt;...&gt;(Cbor::Major, unsigned int, std::span&lt;...&gt;&amp;)</a></summary>.text.bool Cbor::encode&lt;unsigned int&gt;(Cbor::Major, unsigned int, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 9B due to input .rodata.commsCcfStartTasks.str1.1
- Increase 4B due to input .text.startup.main
- Increase 2B due to input .text.Cobs::Decoder::get(unsigned char) const
- Increase 2B due to input .text._raise_r
- Decrease 2B due to input .rodata.vTaskStartScheduler.str1.1

Increase 2.09KiB due to output .bss

- Add 2.00KiB due to input .bss.commsCcfStack
- Add 88B due to input .bss.commsCcfTcb
- Add 4B due to input .bss.rxTask

## Comparing FreeRTOS-Demo-1.elf.map to FreeRTOS-Demo-2.elf.map
Increase 913B due to output .text

- Add 404B due to input <details style="display:inline-block;"><summary><a>.text.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;)</a></summary>.text.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt; &gt; const&amp;)</details>
- Decrease 344B due to input .text.commsCcfProcessTask(void*)
- Add 158B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::unpack(std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::unpack(std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 128B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::call(std::span&lt;...&gt;&amp;, std::span&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;::call(std::span&lt;unsigned char, 4294967295u&gt;&amp;, std::span&lt;unsigned char, 4294967295u&gt;&amp;) const</details>
- Add 104B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::schema(Cbor::Sequence&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;::schema(Cbor::Sequence&lt;(Cbor::Major)4&gt;&amp;) const</details>
- Add 104B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::Cbor&lt;...&gt;::decode(std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::Cbor&lt;std::tuple&lt;&gt; &gt;::decode(std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 84B due to input <details style="display:inline-block;"><summary><a>.text.bool (anonymous namespace)::pack&lt;...&gt;(Cbor::Major, unsigned long long, std::span&lt;...&gt;&amp;)</a></summary>.text.bool (anonymous namespace)::pack&lt;unsigned long long&gt;(Cbor::Major, unsigned long long, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 68B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::Cbor&lt;...&gt;::encode(std::basic_string_view&lt;...&gt;, std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::Cbor&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt; &gt;::encode(std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 48B due to input .text.rpc::{lambda()#1}::_FUN()
- Add 41B due to input <details style="display:inline-block;"><summary><a>.rodata.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;) [clone .str1.1]</a></summary>.rodata.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt; &gt; const&amp;) [clone .str1.1]</details>
- Remove 41B due to input .rodata.commsCcfProcessTask(void*) [clone .str1.1]
- Increase 40B due to input .text.startup._GLOBAL__sub_I_ccf
- Increase 28B due to input .rodata.IntDefaultHandler.str1.1
- Add 25B due to input .rodata._GLOBAL__sub_I_ccf.str1.1
- Add 24B due to input <details style="display:inline-block;"><summary><a>.text.bool Cbor::encode&lt;...&gt;(Cbor::Major, unsigned long long, std::span&lt;...&gt;&amp;)</a></summary>.text.bool Cbor::encode&lt;unsigned long long&gt;(Cbor::Major, unsigned long long, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 24B due to input <details style="display:inline-block;"><summary><a>.rodata.Type&lt;...&gt;::python</a></summary>.rodata.Type&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt; &gt;::python</details>
- Add 18B due to input .text
- Add 16B due to input <details style="display:inline-block;"><summary><a>.rodata.vtable for Call&lt;...&gt;</a></summary>.rodata.vtable for Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;</details>
- Add 11B due to input .rodata.rpc::{lambda()#1}::_FUN() [clone .str1.1]
- Increase 4B due to input <details style="display:inline-block;"><summary><a>.text.Ccf&lt;...&gt;::send(Channels, std::span&lt;...&gt;&amp;)</a></summary>.text.Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::send(Channels, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Decrease 2B due to input .text.Cobs::Decoder::get(unsigned char) const
- Decrease 2B due to input .text.memcpy

Increase 24B due to output .bss

- Add 24B due to input .bss.rpc

## Comparing FreeRTOS-Demo-2.elf.map to FreeRTOS-Demo-3.elf.map
Increase 729B due to output .text

- Add 420B due to input <details style="display:inline-block;"><summary><a>.text.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;)</a></summary>.text.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; const&amp;)</details>
- Remove 404B due to input <details style="display:inline-block;"><summary><a>.text.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;)</a></summary>.text.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt; &gt; const&amp;)</details>
- Add 252B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::schema(Cbor::Sequence&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;unsigned long, unsigned long, unsigned long&gt;::schema(Cbor::Sequence&lt;(Cbor::Major)4&gt;&amp;) const</details>
- Add 160B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::Cbor&lt;...&gt;::decode(std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::Cbor&lt;std::tuple&lt;unsigned long, unsigned long&gt; &gt;::decode(std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 60B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::Cbor&lt;...&gt;::decode(std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::Cbor&lt;unsigned long&gt;::decode(std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 54B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::call(std::span&lt;...&gt;&amp;, std::span&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;unsigned long, unsigned long, unsigned long&gt;::call(std::span&lt;unsigned char, 4294967295u&gt;&amp;, std::span&lt;unsigned char, 4294967295u&gt;&amp;) const</details>
- Increase 52B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::schema(Cbor::Sequence&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;::schema(Cbor::Sequence&lt;(Cbor::Major)4&gt;&amp;) const</details>
- Increase 52B due to input .text.startup._GLOBAL__sub_I_ccf
- Add 41B due to input <details style="display:inline-block;"><summary><a>.rodata.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;) [clone .str1.1]</a></summary>.rodata.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; const&amp;) [clone .str1.1]</details>
- Remove 41B due to input <details style="display:inline-block;"><summary><a>.rodata.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;) [clone .str1.1]</a></summary>.rodata.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt; &gt; const&amp;) [clone .str1.1]</details>
- Add 28B due to input <details style="display:inline-block;"><summary><a>.text.bool Cbor::Sequence&lt;...&gt;::encode&lt;...&gt;(std::basic_string_view&lt;...&gt;)</a></summary>.text.bool Cbor::Sequence&lt;(Cbor::Major)4&gt;::encode&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt; &gt;(std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;)</details>
- Add 26B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::Sequence&lt;...&gt;::~Sequence()</a></summary>.text.Cbor::Sequence&lt;(Cbor::Major)4&gt;::~Sequence()</details>
- Increase 19B due to input .rodata._GLOBAL__sub_I_ccf.str1.1
- Increase 17B due to input .rodata.IntDefaultHandler.str1.1
- Add 16B due to input <details style="display:inline-block;"><summary><a>.rodata.vtable for Call&lt;...&gt;</a></summary>.rodata.vtable for Call&lt;unsigned long, unsigned long, unsigned long&gt;</details>
- Add 6B due to input <details style="display:inline-block;"><summary><a>.rodata.Type&lt;...&gt;::python</a></summary>.rodata.Type&lt;unsigned long&gt;::python</details>
- Decrease 4B due to input <details style="display:inline-block;"><summary><a>.text.Ccf&lt;...&gt;::send(Channels, std::span&lt;...&gt;&amp;)</a></summary>.text.Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::send(Channels, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 4B due to input .text.rpc::{lambda(unsigned long, unsigned long)#1}::_FUN(unsigned long, unsigned long)
- Add 4B due to input <details style="display:inline-block;"><summary><a>.text.bool Cbor::encode&lt;...&gt;(Cbor::Major, unsigned long, std::span&lt;...&gt;&amp;)</a></summary>.text.bool Cbor::encode&lt;unsigned long&gt;(Cbor::Major, unsigned long, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Decrease 3B due to input <details style="display:inline-block;"><summary><a>.rodata.Type&lt;...&gt;::python</a></summary>.rodata.Type&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt; &gt;::python</details>
- Decrease 2B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::Cbor&lt;...&gt;::encode(std::basic_string_view&lt;...&gt;, std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::Cbor&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt; &gt;::encode(std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Increase 2B due to input <details style="display:inline-block;"><summary><a>.text.std::_Optional_payload_base&lt;...&gt;::_M_reset()</a></summary>.text.std::_Optional_payload_base&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt;::Frame&gt;::_M_reset()</details>
- Decrease 2B due to input <details style="display:inline-block;"><summary><a>.text.std::__conditional&lt;...&gt;::type&lt;...&gt; std::ranges::__copy_or_move&lt;...&gt;(char const*, char const*, std::back_insert_iterator&lt;...&gt;)</a></summary>.text.std::__conditional&lt;false&gt;::type&lt;std::ranges::in_out_result&lt;char const*, std::back_insert_iterator&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt; &gt; &gt;, std::ranges::in_out_result&lt;char const*, std::back_insert_iterator&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt; &gt; &gt; &gt; std::ranges::__copy_or_move&lt;false, char const*, char const*, std::back_insert_iterator&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt; &gt; &gt;(char const*, char const*, std::back_insert_iterator&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt; &gt;)</details>
- Decrease 2B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::call(std::span&lt;...&gt;&amp;, std::span&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;::call(std::span&lt;unsigned char, 4294967295u&gt;&amp;, std::span&lt;unsigned char, 4294967295u&gt;&amp;) const</details>

Increase 28B due to output .bss

- Increase 28B due to input .bss.rpc

## Comparing FreeRTOS-Demo-3.elf.map to FreeRTOS-Demo-4.elf.map
Increase 65B due to output .text

- Add 420B due to input <details style="display:inline-block;"><summary><a>.text.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;)</a></summary>.text.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; const&amp;)</details>
- Remove 420B due to input <details style="display:inline-block;"><summary><a>.text.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;)</a></summary>.text.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; const&amp;)</details>
- Add 41B due to input <details style="display:inline-block;"><summary><a>.rodata.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;) [clone .str1.1]</a></summary>.rodata.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; const&amp;) [clone .str1.1]</details>
- Remove 41B due to input <details style="display:inline-block;"><summary><a>.rodata.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;) [clone .str1.1]</a></summary>.rodata.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; const&amp;) [clone .str1.1]</details>
- Increase 36B due to input .text.startup._GLOBAL__sub_I_ccf
- Increase 15B due to input .rodata.IntDefaultHandler.str1.1
- Increase 15B due to input .rodata._GLOBAL__sub_I_ccf.str1.1
- Add 8B due to input .text.rpc::{lambda(unsigned long, unsigned long)#2}::_FUN(unsigned long, unsigned long)
- Increase 1B due to input <details style="display:inline-block;"><summary><a>.rodata.Type&lt;...&gt;::python</a></summary>.rodata.Type&lt;unsigned long&gt;::python</details>

Increase 28B due to output .bss

- Increase 28B due to input .bss.rpc

## Comparing FreeRTOS-Demo-4.elf.map to FreeRTOS-Demo-5.elf.map
Increase 304B due to output .text

- Add 420B due to input <details style="display:inline-block;"><summary><a>.text.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;)</a></summary>.text.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt; &gt; const&amp;)</details>
- Remove 420B due to input <details style="display:inline-block;"><summary><a>.text.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;)</a></summary>.text.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; const&amp;)</details>
- Add 152B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::schema(Cbor::Sequence&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt;::schema(Cbor::Sequence&lt;(Cbor::Major)4&gt;&amp;) const</details>
- Add 48B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::call(std::span&lt;...&gt;&amp;, std::span&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt;::call(std::span&lt;unsigned char, 4294967295u&gt;&amp;, std::span&lt;unsigned char, 4294967295u&gt;&amp;) const</details>
- Add 41B due to input <details style="display:inline-block;"><summary><a>.rodata.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;) [clone .str1.1]</a></summary>.rodata.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt; &gt; const&amp;) [clone .str1.1]</details>
- Remove 41B due to input <details style="display:inline-block;"><summary><a>.rodata.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;) [clone .str1.1]</a></summary>.rodata.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt; &gt; const&amp;) [clone .str1.1]</details>
- Increase 40B due to input .text.startup._GLOBAL__sub_I_ccf
- Increase 24B due to input .rodata.IntDefaultHandler.str1.1
- Add 20B due to input .text.rpc::{lambda()#2}::_FUN()
- Add 16B due to input <details style="display:inline-block;"><summary><a>.rodata.vtable for Call&lt;...&gt;</a></summary>.rodata.vtable for Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt;</details>
- Increase 12B due to input .rodata._GLOBAL__sub_I_ccf.str1.1
- Add 12B due to input .rodata.str1.1
- Add 8B due to input .rodata
- Decrease 4B due to input .text.rpc::{lambda(unsigned long, unsigned long)#2}::_FUN(unsigned long, unsigned long)
- Add 4B due to input <details style="display:inline-block;"><summary><a>.rodata.Type&lt;...&gt;::python</a></summary>.rodata.Type&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt; &gt;::python</details>

Increase 24B due to output .bss

- Increase 24B due to input .bss.rpc

## Comparing FreeRTOS-Demo-5.elf.map to FreeRTOS-Demo-6.elf.map
Increase 1.39KiB due to output .text

- Add 420B due to input <details style="display:inline-block;"><summary><a>.text.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;)</a></summary>.text.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt;, Call&lt;std::span&lt;unsigned char, 4294967295u&gt;, unsigned int, unsigned int&gt;, Call&lt;void, unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt;, Call&lt;std::span&lt;unsigned char, 4294967295u&gt;, unsigned int, unsigned int&gt;, Call&lt;void, unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt; &gt; const&amp;)</details>
- Remove 420B due to input <details style="display:inline-block;"><summary><a>.text.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;)</a></summary>.text.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt; &gt; const&amp;)</details>
- Add 264B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::schema(Cbor::Sequence&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;void, unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt;::schema(Cbor::Sequence&lt;(Cbor::Major)4&gt;&amp;) const</details>
- Add 260B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::schema(Cbor::Sequence&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;std::span&lt;unsigned char, 4294967295u&gt;, unsigned int, unsigned int&gt;::schema(Cbor::Sequence&lt;(Cbor::Major)4&gt;&amp;) const</details>
- Add 230B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::Cbor&lt;...&gt;::decode(std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::Cbor&lt;std::tuple&lt;unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt; &gt;::decode(std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Add 160B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::Cbor&lt;...&gt;::decode(std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::Cbor&lt;std::tuple&lt;unsigned int, unsigned int&gt; &gt;::decode(std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Increase 112B due to input .text.startup._GLOBAL__sub_I_ccf
- Add 108B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::call(std::span&lt;...&gt;&amp;, std::span&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;std::span&lt;unsigned char, 4294967295u&gt;, unsigned int, unsigned int&gt;::call(std::span&lt;unsigned char, 4294967295u&gt;&amp;, std::span&lt;unsigned char, 4294967295u&gt;&amp;) const</details>
- Add 60B due to input <details style="display:inline-block;"><summary><a>.text.Cbor::Cbor&lt;...&gt;::decode(std::span&lt;...&gt;&amp;)</a></summary>.text.Cbor::Cbor&lt;unsigned int&gt;::decode(std::span&lt;unsigned char, 4294967295u&gt;&amp;)</details>
- Increase 59B due to input .rodata.IntDefaultHandler.str1.1
- Increase 59B due to input .rodata._GLOBAL__sub_I_ccf.str1.1
- Add 52B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::call(std::span&lt;...&gt;&amp;, std::span&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;void, unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt;::call(std::span&lt;unsigned char, 4294967295u&gt;&amp;, std::span&lt;unsigned char, 4294967295u&gt;&amp;) const</details>
- Add 41B due to input <details style="display:inline-block;"><summary><a>.rodata.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;) [clone .str1.1]</a></summary>.rodata.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt;, Call&lt;std::span&lt;unsigned char, 4294967295u&gt;, unsigned int, unsigned int&gt;, Call&lt;void, unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt;, Call&lt;std::span&lt;unsigned char, 4294967295u&gt;, unsigned int, unsigned int&gt;, Call&lt;void, unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt; &gt; const&amp;) [clone .str1.1]</details>
- Remove 41B due to input <details style="display:inline-block;"><summary><a>.rodata.bool Ccf&lt;...&gt;::poll&lt;...&gt;(Rpc&lt;...&gt; const&amp;) [clone .str1.1]</a></summary>.rodata.bool Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::poll&lt;Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt; &gt; &gt;(Rpc&lt;Call&lt;std::tuple&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;, unsigned long long&gt;&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;unsigned long, unsigned long, unsigned long&gt;, Call&lt;std::basic_string_view&lt;char, std::char_traits&lt;char&gt; &gt;&gt; &gt; const&amp;) [clone .str1.1]</details>
- Add 36B due to input <details style="display:inline-block;"><summary><a>.text.std::optional&lt;...&gt; Cbor::Cbor&lt;...&gt;::transpose&lt;...&gt;(std::tuple&lt;...&gt;)</a></summary>.text.std::optional&lt;std::tuple&lt;unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt; &gt; Cbor::Cbor&lt;std::tuple&lt;unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt; &gt;::transpose&lt;unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt;(std::tuple&lt;std::optional&lt;unsigned int&gt;, std::optional&lt;std::span&lt;unsigned char, 4294967295u&gt; &gt; &gt;)</details>
- Add 30B due to input <details style="display:inline-block;"><summary><a>.text.rpc::{lambda(unsigned int, std::span&lt;...&gt;)#1}::_FUN(unsigned int, std::span&lt;...&gt;)</a></summary>.text.rpc::{lambda(unsigned int, std::span&lt;unsigned char, 4294967295u&gt;)#1}::_FUN(unsigned int, std::span&lt;unsigned char, 4294967295u&gt;)</details>
- Add 16B due to input <details style="display:inline-block;"><summary><a>.rodata.vtable for Call&lt;...&gt;</a></summary>.rodata.vtable for Call&lt;std::span&lt;unsigned char, 4294967295u&gt;, unsigned int, unsigned int&gt;</details>
- Add 16B due to input <details style="display:inline-block;"><summary><a>.rodata.vtable for Call&lt;...&gt;</a></summary>.rodata.vtable for Call&lt;void, unsigned int, std::span&lt;unsigned char, 4294967295u&gt; &gt;</details>
- Add 8B due to input .text.rpc::{lambda(unsigned int, unsigned int)#1}::_FUN(unsigned int, unsigned int)
- Add 6B due to input <details style="display:inline-block;"><summary><a>.rodata.Type&lt;...&gt;::python</a></summary>.rodata.Type&lt;std::span&lt;unsigned char, 4294967295u&gt; &gt;::python</details>
- Add 6B due to input <details style="display:inline-block;"><summary><a>.rodata.Type&lt;...&gt;::python</a></summary>.rodata.Type&lt;void&gt;::python</details>
- Add 4B due to input <details style="display:inline-block;"><summary><a>.rodata.Type&lt;...&gt;::python</a></summary>.rodata.Type&lt;unsigned int&gt;::python</details>
- Decrease 3B due to input <details style="display:inline-block;"><summary><a>.rodata.Type&lt;...&gt;::python</a></summary>.rodata.Type&lt;unsigned long&gt;::python</details>
- Decrease 2B due to input <details style="display:inline-block;"><summary><a>.text.std::_Optional_payload_base&lt;...&gt;::_M_reset()</a></summary>.text.std::_Optional_payload_base&lt;CircularBuffer&lt;unsigned char, 256u, 255u&gt;::Frame&gt;::_M_reset()</details>
- Increase 2B due to input <details style="display:inline-block;"><summary><a>.text.Call&lt;...&gt;::call(std::span&lt;...&gt;&amp;, std::span&lt;...&gt;&amp;) const</a></summary>.text.Call&lt;unsigned long, unsigned long, unsigned long&gt;::call(std::span&lt;unsigned char, 4294967295u&gt;&amp;, std::span&lt;unsigned char, 4294967295u&gt;&amp;) const</details>

Increase 56B due to output .bss

- Increase 56B due to input .bss.rpc

## Comparing FreeRTOS-Demo-6.elf.map to FreeRTOS-Demo-7.elf.map
Increase 4.10KiB due to output .text

- Add 588B due to input .text._printf_i
- Add 552B due to input .text._vfprintf_r
- Add 496B due to input .text._svfprintf_r
- Add 336B due to input .text.xQueueSemaphoreTake
- Increase 244B due to input .rodata.IntDefaultHandler.str1.1
- Add 228B due to input .text._printf_common
- Add 192B due to input .text.__ssputs_r
- Add 172B due to input .text.vTaskPriorityDisinheritAfterTimeout
- Add 168B due to input .text.__swsetup_r
- Add 160B due to input .text.xTaskPriorityInherit
- Add 129B due to input .rodata.logTask(void*) [clone .str1.1]
- Add 126B due to input .text.__swbuf_r
- Add 114B due to input .text.__smakebuf_r
- Add 112B due to input .text.logTask(void*)
- Add 92B due to input .text._vsnprintf_r
- Add 92B due to input .text._realloc_r
- Add 80B due to input <details style="display:inline-block;"><summary><a>.text.Ccf&lt;...&gt;::log(LogLevel, unsigned char, char const*, ...)</a></summary>.text.Ccf&lt;CcfConfig{256u, 256u, 255u}&gt;::log(LogLevel, unsigned char, char const*, ...)</details>
- Add 76B due to input .text.vTaskDelay
- Add 61B due to input .rodata.__assert_func.str1.1
- Add 60B due to input .text.__assert_func
- Add 48B due to input .text.createLogTask
- Add 42B due to input .text.__sfputc_r
- Add 38B due to input .text.__sfputs_r
- Add 36B due to input .text.fprintf
- Add 34B due to input .rodata._printf_i.str1.1
- Add 32B due to input .text._isatty_r
- Add 28B due to input .text.vsnprintf
- Add 28B due to input .text.memchr
- Add 20B due to input .text.pvTaskIncrementMutexHeldCount
- Add 17B due to input .rodata._svfprintf_r.str1.1
- Add 17B due to input .rodata._vfprintf_r.str1.1
- Add 16B due to input .text._malloc_usable_size_r
- Add 6B due to input .text.unlikely._isatty
- Increase 4B due to input .text.startup.main
- Increase 4B due to input .text.rpc::{lambda(unsigned int, unsigned int)#1}::_FUN(unsigned int, unsigned int)
- Add 4B due to input .rodata.createLogTask.str1.1
- Increase 2B due to input .text.unlikely._read

Increase 1.09KiB due to output .bss

- Add 1024B due to input .bss.logStack
- Add 88B due to input .bss.logTcb
