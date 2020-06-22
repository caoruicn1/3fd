//
// Copyright (c) 2020 Part of 3FD project (https://github.com/faburaya/3fd)
// It is FREELY distributed by the author under the Microsoft Public License
// and the observance that it should only be used for the benefit of mankind.
//
#include "pch.h"
#include <3fd/utils/cmdline.h>
#include <string>

namespace _3fd
{
namespace unit_tests
{
    /// <summary>
    /// Helps creating a list of writeable C-Style string out of read-only literals.
    /// </summary>
    class ListOfArguments
    {
    private:
        
        std::vector<std::string> m_strings;

    public:

        ListOfArguments(const std::vector<const char *> &readOnlyArgs)
        {
            m_strings.reserve(readOnlyArgs.size());
            for (auto roarg : readOnlyArgs)
            {
                m_strings.push_back(std::string(roarg));
            }
        }

        std::vector<char *> GetList()
        {
            std::vector<char *> list;
            list.reserve(m_strings.size() + 1);
            for (auto &arg : m_strings)
            {
                list.push_back(arg.data());
            }
            list.push_back(nullptr); // always at the end

            return list;
        }
    }; // end of class ListOfArguments

    using _3fd::core::CommandLineArguments;

    struct CommandLineParserTestParams
    {
        CommandLineArguments::ArgOptionSign optionSign;
        CommandLineArguments::ArgValSeparator valueSeparator;
        std::vector<const char *> args;
    };

    using Params = CommandLineParserTestParams;

    class CommandLineParserTestWithParameters
        : public ::testing::TestWithParam<Params> {};

    using Parameterized = CommandLineParserTestWithParameters;

    TEST_P(Parameterized, WithOneParamNumber)
    {
        auto params = GetParam();
        CommandLineArguments cmdLineArgs(120, params.optionSign, params.valueSeparator, false);

        enum
        {
            ArgValFloatWithinRange
        };

        cmdLineArgs.AddExpectedArgument(
            CommandLineArguments::ArgDeclaration{
                ArgValFloatWithinRange,
                CommandLineArguments::ArgType::OptionWithReqValue,
                CommandLineArguments::ArgValType::RangeFloat,
                'n', "number",
                "a floating point value from 0 to 1"
            },
            { 0.5, 0.0, 1.0 }
        );

        struct
        {
            struct
            {
                float number;
            } expected, actual;

            std::vector<char *> args;
        } test;

        ListOfArguments args(params.args);
        test.args = args.GetList();
        test.expected.number = 0.5F;

        bool status = cmdLineArgs.Parse(test.args.size() - 1, test.args.data());
        EXPECT_TRUE(status == STATUS_OKAY);
        if (status == STATUS_FAIL)
        {
            std::cerr << "Expected usage:\n";
            cmdLineArgs.PrintArgsInfo();
            FAIL();
        }

        bool isPresent;
        test.actual.number = cmdLineArgs.GetArgValueFloat(ArgValFloatWithinRange, isPresent);
        EXPECT_TRUE(isPresent);
        EXPECT_EQ(test.expected.number, test.actual.number);
    }

    INSTANTIATE_TEST_CASE_P(CommandLineParserTest,
                            Parameterized,
                            ::testing::Values(
                                Params{ CommandLineArguments::ArgOptionSign::Dash,
                                        CommandLineArguments::ArgValSeparator::Colon,
                                        { "program.exe", "-n:0.5" } },
                                Params{ CommandLineArguments::ArgOptionSign::Dash,
                                        CommandLineArguments::ArgValSeparator::Colon,
                                        { "program.exe", "--number:0.5" } },
                                Params{ CommandLineArguments::ArgOptionSign::Slash,
                                        CommandLineArguments::ArgValSeparator::Colon,
                                        { "program.exe", "/n:0.5" } },
                                Params{ CommandLineArguments::ArgOptionSign::Slash,
                                        CommandLineArguments::ArgValSeparator::Colon,
                                        { "program.exe", "/number:0.5" } }
                            ));

    TEST(CommandLineParser, ColonAsValSeparator_OneParam_EnumOptions)
    {
        using _3fd::core::CommandLineArguments;

        CommandLineArguments cmdLineArgs(120,
                                         CommandLineArguments::ArgOptionSign::Dash,
                                         CommandLineArguments::ArgValSeparator::Colon,
                                         false);
        enum
        {
            ArgValFromEnumStrOptions
        };

        cmdLineArgs.AddExpectedArgument(CommandLineArguments::ArgDeclaration{
            ArgValFromEnumStrOptions,
            CommandLineArguments::ArgType::OptionWithReqValue,
            CommandLineArguments::ArgValType::EnumString,
            'o', "option",
            "an option from a list of possible strings"
        }, { "option1", "option2" });

        struct
        {
            struct
            {
                const char *chosenOption;
            } expected, actual;

            std::vector<char *> args;
        } test;

        ListOfArguments args({ "program.exe", "-o:option1" });
        test.args = args.GetList();

        test.expected.chosenOption = "option1";

        bool status = cmdLineArgs.Parse(test.args.size() - 1, test.args.data());
        EXPECT_TRUE(status == STATUS_OKAY);
        if (status == STATUS_FAIL)
        {
            std::cerr << "Expected usage:\n";
            cmdLineArgs.PrintArgsInfo();
            FAIL();
        }

        bool isPresent;
        test.actual.chosenOption = cmdLineArgs.GetArgValueString(ArgValFromEnumStrOptions, isPresent);
        EXPECT_TRUE(isPresent);
        EXPECT_STREQ(test.expected.chosenOption, test.actual.chosenOption);
    }

    TEST(CommandLineParser, ColonAsValSeparator_List)
    {
        using _3fd::core::CommandLineArguments;

        CommandLineArguments cmdLineArgs(120,
                                         CommandLineArguments::ArgOptionSign::Dash,
                                         CommandLineArguments::ArgValSeparator::Colon,
                                         false);
        enum
        {
            ArgValsListOfStrings
        };

        cmdLineArgs.AddExpectedArgument(CommandLineArguments::ArgDeclaration{
            ArgValsListOfStrings,
            CommandLineArguments::ArgType::ValuesList,
            CommandLineArguments::ArgValType::String,
            0, "name1 name2",
            "list with two names"
        }, { (uint16_t)2, (uint16_t)2 });

        struct
        {
            struct
            {
                std::vector<const char *> names;
            } expected, actual;

            std::vector<char *> args;
        } test;

        ListOfArguments args({ "program.exe", "ping", "pong" });
        test.args = args.GetList();
        test.expected.names = { "ping", "pong" };

        bool status = cmdLineArgs.Parse(test.args.size() - 1, test.args.data());
        EXPECT_TRUE(status == STATUS_OKAY);
        if (status == STATUS_FAIL)
        {
            std::cerr << "Expected usage:\n";
            cmdLineArgs.PrintArgsInfo();
            FAIL();
        }

        cmdLineArgs.GetArgListOfValues(test.actual.names);
        ASSERT_EQ(2, test.actual.names.size());

        for (int idx = 0; idx < test.actual.names.size(); ++idx)
            EXPECT_STREQ(test.expected.names[idx], test.actual.names[idx]);
    }

    TEST(CommandLineParser, ColonAsValSeparator_CombinedUsage)
    {
        using _3fd::core::CommandLineArguments;

        CommandLineArguments cmdLineArgs(120,
                                         CommandLineArguments::ArgOptionSign::Dash,
                                         CommandLineArguments::ArgValSeparator::Colon,
                                         false);
        enum
        {
            ArgValFromEnumStrOptions, ArgValFloatWithinRange, ArgValsListOfStrings
        };

        cmdLineArgs.AddExpectedArgument(CommandLineArguments::ArgDeclaration{
            ArgValFromEnumStrOptions,
            CommandLineArguments::ArgType::OptionWithReqValue,
            CommandLineArguments::ArgValType::EnumString,
            'o', "option",
            "an option from a list of possible strings"
        },
        { "option1", "option2" });

        cmdLineArgs.AddExpectedArgument(CommandLineArguments::ArgDeclaration{
            ArgValFloatWithinRange,
            CommandLineArguments::ArgType::OptionWithReqValue,
            CommandLineArguments::ArgValType::RangeFloat,
            'n', "number",
            "a floating point value from 0 to 1"
        },
        { 0.5, 0.0, 1.0 });

        cmdLineArgs.AddExpectedArgument(CommandLineArguments::ArgDeclaration{
            ArgValsListOfStrings,
            CommandLineArguments::ArgType::ValuesList,
            CommandLineArguments::ArgValType::String,
            0, "name1 name2",
            "list with two names"
        }, { (uint16_t)2, (uint16_t)2 });

        struct
        {
            struct
            {
                const char *chosenOption;
                float number;
                std::vector<const char *> names;
            } expected, actual;

            std::vector<char *> args;
        } test;

        ListOfArguments args({ "program.exe", "-o:option1", "-n:0.5", "ping", "pong" });
        test.args = args.GetList();

        test.expected.chosenOption = "option1";
        test.expected.number = 0.5F;
        test.expected.names = { "ping", "pong" };

        bool status = cmdLineArgs.Parse(test.args.size() - 1, test.args.data());
        EXPECT_TRUE(status == STATUS_OKAY);
        if (status == STATUS_FAIL)
        {
            std::cerr << "Expected usage:\n";
            cmdLineArgs.PrintArgsInfo();
            FAIL();
        }

        bool isPresent;
        test.actual.chosenOption = cmdLineArgs.GetArgValueString(ArgValFromEnumStrOptions, isPresent);
        EXPECT_TRUE(isPresent);
        EXPECT_STREQ(test.expected.chosenOption, test.actual.chosenOption);

        test.actual.number = cmdLineArgs.GetArgValueFloat(ArgValFloatWithinRange, isPresent);
        EXPECT_TRUE(isPresent);
        EXPECT_EQ(test.expected.number, test.actual.number);

        cmdLineArgs.GetArgListOfValues(test.actual.names);
        ASSERT_EQ(2, test.actual.names.size());

        for (int idx = 0; idx < test.actual.names.size(); ++idx)
            EXPECT_STREQ(test.expected.names[idx], test.actual.names[idx]);
    }

} // end of namespace unit_tests
} // end of namespace _3fd